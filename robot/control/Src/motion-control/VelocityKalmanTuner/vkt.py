import datetime
import os
import sys

from argparse import ArgumentParser

import numpy as np
import matplotlib.pyplot as plt
from PyQt5.QtWidgets import QApplication, QFileDialog

from UI import UIMainWindow
from KalmanFilter import KalmanFilter

'''
Credit: https://www.bzarg.com/p/how-a-kalman-filter-works-in-pictures/
https://en.wikipedia.org/wiki/Kalman_filter
'''


class Main(QApplication):
    def __init__(self, args, kalmanFilter, verbose=False):
        super(Main, self).__init__([])
        self.args = args
        self.kalmanFilter = kalmanFilter

        if self.args.time:
            try:
                self.t_total = float(self.args.time)
            except ValueError:
                print("Argument 'time' must be a number!")
                self.terminate()
        else:
            self.t_total = 2.0

        if self.args.timestep:
            try:
                self.dt = float(self.args.timestep)
            except ValueError:
                print("Argument 'timestep' must be a number!")
                self.terminate()
        else:
            self.dt = 0.005

        self.verbose = self.args.verbose

        if self.args.headless:
            self.kalmanFilter.steady_state = not self.args.regular

        if not self.args.headless:
            self.window = UIMainWindow()
            self.populate_gains()
            self.window.sim_bttn.clicked.connect(self.simulate)
            self.window.gen_bttn.clicked.connect(self.generate_code)
            self.window.save_action.triggered.connect(self.save)
            self.window.load_action.triggered.connect(self.load)
            self.window.filter_type_select.setCurrentText(
                "Steady State" if self.kalmanFilter.steady_state else "Regular")
            self.window.filter_type_select.currentTextChanged.connect(self.set_filter_type)
            self.window.data_type_select.currentTextChanged.connect(self.set_data_type)
            self.window.verbose_check.stateChanged.connect(self.set_verbose)
            self.window.show()
        self.ts, self.vxs, self.vys, self.omegas = np.arange(0, self.t_total, self.dt), [], [], []
        self.vxs_obs, self.vys_obs, self.omegas_obs = [], [], []
        self.lists = [self.vxs, self.vys, self.omegas, self.vxs_obs, self.vys_obs, self.omegas_obs]

        if self.args.headless:
            # Check arguments:

            # Loadfile with gains must be a valid .txt file
            if self.args.loadfile:
                if not os.path.isfile(os.path.abspath(self.args.loadfile)):
                    print("Error: Argument provided to -l/--loadfile is a not a valid path!")
                    self.terminate()
                if os.path.splitext(self.args.loadfile)[1] != ".txt":
                    print("Error: Argument provided to -l/--loadfile must be a .txt file!")
                    self.terminate()

            # Savefile must be a valid.txt file in a valid directory
            if self.args.savefile:
                if not os.path.isdir(os.path.split(os.path.abspath(self.args.savefile))[0]):
                    print("Error: Argument provided to -s/--savefile is not a valid path!")
                    self.terminate()

                if os.path.splitext(self.args.savefile)[1] != ".txt":
                    print("Error: Argument provided to -s/--savefile must be a .txt file!")
                    self.terminate()

            # Outfile must be a valid .hpp file in a valid directory
            if self.args.outfile:
                if not os.path.isdir(os.path.split(os.path.abspath(self.args.outfile))[0]):
                    print("Error: Argument provided to -o/--outfile is not a valid path!")
                    self.terminate()

                if os.path.splitext(self.args.outfile)[1] != ".hpp":
                    print("Error: Argument provided to -o/--outfile must be a .hpp file!")
                    self.terminate()

            if self.args.loadfile:
                self.load()

            self.simulate()

            if self.args.savefile:
                self.save()

            if self.args.outfile:
                self.generate_code()

            self.terminate()

    def terminate(self):
        self.quit()
        sys.exit(0)

    def populate_gains(self):
        for r, row in enumerate(self.kalmanFilter.Q):
            for c, element in enumerate(row):
                self.window.Q_textboxes[r][c].setText(str(self.kalmanFilter.Q[r, c]))

        for r, row in enumerate(self.kalmanFilter.R):
            for c, element in enumerate(row):
                self.window.R_textboxes[r][c].setText(str(self.kalmanFilter.R[r, c]))

    def update(self):
        t, x_hat, x = self.kalmanFilter.step(self.dt, self.verbose, changeable_only=True)
        vx, vy, omega = x_hat
        vx_obs, vy_obs, omega_obs = x
        self.vxs.append(vx)
        self.vys.append(vy)
        self.omegas.append(omega)
        self.vxs_obs.append(vx_obs)
        self.vys_obs.append(vy_obs)
        self.omegas_obs.append(omega_obs)
        if self.verbose:
            self.console_print("-" * 40)
            v = self.kalmanFilter.vars(True)
            for key in v:
                self.console_print(str(key) + ":\n" + str(np.round(v[key], 5)))

    def set_filter_type(self, new_option):
        self.kalmanFilter.steady_state = (new_option == "Steady State")

    def set_data_type(self, new_option):
        self.kalmanFilter.step_response = (new_option == "Step Response")
        if new_option == "Sensor Data":
            file_name = \
            QFileDialog.getOpenFileName(self.window.central_widget, "Import Sensor Data", '', "CSV file (*.csv)")[0]
            with open(file_name, "r") as file:
                data = file.read()
                print(data.split("\n"))
                if len(data.split("\n")) != self.kalmanFilter.num_states + 1:
                    print("Make sure that there are {} rows of data: 1 for sample frequency, {} for states".format(
                        self.kalmanFilter.num_states + 1, self.kalmanFilter.num_states))
                if len(data.split("\n")[0].split(",")) != 1:
                    print("The first line of the file must be the sample rate")
                try:
                    float(data.split("\n")[0])
                except ValueError:
                    print("The sample rate must be a number!")
                lengths = [len(row.split(",")) for row in data.split("\n")[1:]]
                if len(set(lengths)) != 1:
                    print("Make sure that each row of sensor data is of the same length!")

    def set_verbose(self):
        if not self.args.headless:
            self.verbose = self.window.verbose_check.isChecked()

    def simulate(self):
        if not self.args.headless:
            self.window.console.clear()
            err = False
            # Validate gains
            for row in range(self.kalmanFilter.num_states):
                for col in range(self.kalmanFilter.num_states):
                    try:
                        self.kalmanFilter.Q[row, col] = eval(self.window.Q_textboxes[row][col].text())
                    except Exception:
                        err = True
                        self.console_print(
                            "Make sure that element {},{} of the Q gains matrix is a valid number!".format(row, col))
                        print("Make sure that element {},{} of the Q gains matrix is a valid number!".format(row, col))

            for row in range(self.kalmanFilter.num_outputs):
                for col in range(self.kalmanFilter.num_outputs):
                    try:
                        self.kalmanFilter.R[row, col] = eval(self.window.R_textboxes[row][col].text())
                    except Exception:
                        err = True
                        self.console_print(
                            "Make sure that element {},{} of the R gains matrix is a valid number!".format(row, col))
                        print("Make sure that element {},{} of the R gains matrix is a valid number!".format(row, col))

            if err:
                return

        for var_list in self.lists:
            var_list.clear()

        self.kalmanFilter.reset()

        for _ in self.ts:
            self.update()

        if self.args.headless:
            if self.args.savefig:
                fig = plt.figure()
                vx_ax = fig.add_subplot(3, 1, 1)
                vy_ax = fig.add_subplot(3, 1, 2)
                omega_ax = fig.add_subplot(3, 1, 3)

                vx_ax.set_title('X velocity over Time')
                vx_ax.set_xlabel('t')
                vx_ax.set_ylabel('vx')

                vy_ax.set_title('Y velocity over Time')
                vy_ax.set_xlabel('t')
                vy_ax.set_ylabel('vy')

                omega_ax.set_title('Omega vs Time')
                omega_ax.set_xlabel('t')
                omega_ax.set_ylabel('\u03C9')  # omega character

                vx_ax.plot(self.ts, self.vxs, '-', c='tab:blue')
                vy_ax.plot(self.ts, self.vys, '-', c='tab:blue')
                omega_ax.plot(self.ts, self.omegas, '-', c='tab:blue')

                vx_ax.plot(self.ts, self.vxs_obs, '-', c='tab:green')
                vy_ax.plot(self.ts, self.vys_obs, '-', c='tab:green')
                omega_ax.plot(self.ts, self.omegas_obs, '-', c='tab:green')

                for ax in [vx_ax, vy_ax, omega_ax]:
                    ax.spines['top'].set_visible(False)
                    ax.spines['right'].set_visible(False)
                    ax.relim()
                    ax.legend(['x_hat', 'x'])

                fig.tight_layout()
                if self.args.savefig != ' ':
                    try:
                        plt.savefig(self.args.savefig)
                    except ValueError as ve:
                        print(ve)
                        self.terminate()
                else:
                    plt.savefig(datetime.datetime.now().strftime('vkt_%m-%d-%Y-%H_%M_%S.png'))

        else:
            self.window.set_plot_data(self.ts,
                                      self.vxs, self.vxs_obs,
                                      self.vys, self.vys_obs,
                                      self.omegas, self.omegas_obs)
            self.window.plot_fig.tight_layout()

    def save(self):
        if self.args.headless and self.args.savefile:
            file_name = self.args.savefile
        else:
            file_name = QFileDialog.getSaveFileName(self.window.central_widget, "Save Gains", '', "Text File (*.txt)")[
                0]
        if file_name:
            with open(file_name, 'w') as file:
                for key, pair in self.kalmanFilter.gains().items():
                    file.write(str(key) + ": " + str(pair.tolist()) + "\n")

    def load(self):
        if self.args.headless and self.args.loadfile:
            file_name = self.args.loadfile
        else:
            file_name = \
            QFileDialog.getOpenFileName(self.window.central_widget, "Import Gains", '', "Text File (*.txt)")[0]
        if file_name:
            with open(file_name, 'r') as file:
                gains_string = file.read()
                P, A, B, H, K, Q, R = [eval(gain.split(': ')[1]) for gain in gains_string.strip().split('\n')]
                self.kalmanFilter.set_gains(P, A, B, H, K, Q, R)
                if not self.args.headless:
                    self.populate_gains()

    def generate_gain(self, name, matrix):
        num_rows, num_cols = matrix.shape
        indent_level = " "*4
        header = indent_level + \
                 "inline static Eigen::Matrix<double, %d, %d> make_%s_matrix() {\n" % (num_rows, num_cols, name)
        rtrn = indent_level + "    return (Eigen::Matrix<double, %d, %d>() << " % (num_rows, num_cols)
        indent = " " * len(rtrn)
        block = rtrn
        for i, row in enumerate(matrix):
            row_str = str(indent) if i != 0 else ""
            for element in row:
                row_str += "%g, " % element
            block += row_str + "\n"
        block = header + block[:-3] + ").finished();\n" + indent_level + "}\n"
        return block

    def generate_code(self):
        if self.args.headless and self.args.outfile:
            file_name = self.args.outfile
        else:
            file_name = QFileDialog.getSaveFileName(self.window.central_widget, "Export C++ Code", '', "C++ (*.hpp)")[0]
        if file_name:
            with open(file_name, 'w') as file:
                file.write("#pragma once\n")
                file.write("#include <Eigen/Dense>\n\n")
                file.write("namespace drivetrain_controls {\n\n")
                gains_list = [self.kalmanFilter.A, self.kalmanFilter.B, self.kalmanFilter.Q, self.kalmanFilter.R,
                              self.kalmanFilter.K]
                names_list = ["dynamics", "control", "process_noise", "observation_noise",
                              "ss_kalman_gain" if self.kalmanFilter.steady_state else "kalman_gain"]
                code = ""
                for name, gain in zip(names_list, gains_list):
                    code += self.generate_gain(name, gain) + "\n"
                file.write(code)
                file.write("}")

    def console_print(self, msg):
        if not self.args.headless:
            self.window.console.insertPlainText(msg + "\n")


if __name__ == '__main__':
    p = ArgumentParser()
    p.add_argument("-l", "--loadfile", dest="loadfile", help="File to load gains")
    p.add_argument("-hd", "--headless", action='store_true', help="Runs without gui")
    p.add_argument("-o", "--outfile", dest="outfile", help="File to export as hpp code")
    p.add_argument("-r", "--regular", action='store_true', help="Runs Kalman filter without steady-state")
    p.add_argument("-f", "--savefig", nargs='?', const=' ', help="Saves figure from simulation to specified file. If blank, "
                                                              "file will be of type PNG and have a datetime string as a name")
    p.add_argument("-s", "--savefile", dest="savefile", help="File to save gains")
    p.add_argument("-t", "--time", dest="time", help="Total time in seconds to run simulation. Default: 2s")
    p.add_argument("-dt", "--timestep", dest="timestep", help="Timestep for simulation, in seconds. Default: 0.005s")
    p.add_argument("-v", "--verbose", action='store_true', help="Prints Kalman Filter gains for every simulation step")
    args = p.parse_args()
    m = Main(args, kalmanFilter=KalmanFilter(x_hat_init=np.array([0, 0, 0]).reshape((3, 1)),
                                             init_covariance=0.1,
                                             steady_state=True),
             verbose=True)
    sys.exit(m.exec_())
