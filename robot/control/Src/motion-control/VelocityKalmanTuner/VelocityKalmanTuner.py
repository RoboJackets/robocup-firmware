import sys
import os
from argparse import ArgumentParser

import numpy as np
from PyQt5.QtWidgets import QApplication, QFileDialog

from functools import partial
from UI import UIMainWindow
from KalmanFilter import KalmanFilter

'''
Credit: https://www.bzarg.com/p/how-a-kalman-filter-works-in-pictures/
https://en.wikipedia.org/wiki/Kalman_filter
https://www.mathworks.com/videos/understanding-kalman-filters-part-4-optimal-state-estimator-algorithm--1493129749201.html
'''

class Main(QApplication):
    def __init__(self, args, kalmanFilter, verbose=False):
        super(Main, self).__init__([])
        self.args = args
        self.kalmanFilter = kalmanFilter
        self.verbose = self.args.verbose
        if not self.args.headless:
            self.window = UIMainWindow()
            self.populate_gains()
            self.window.sim_bttn.clicked.connect(self.simulate)
            self.window.gen_bttn.clicked.connect(self.generate_code)
            self.window.save_action.triggered.connect(self.save)
            self.window.load_action.triggered.connect(self.load)
            self.window.filter_type_select.setCurrentText("Steady State" if self.kalmanFilter.steady_state else "Regular")
            self.window.filter_type_select.currentTextChanged.connect(self.set_filter_type)
            self.window.show()
        self.ts, self.vxs, self.vys, self.omegas = np.arange(0, 10, 0.01), [], [], []
        self.vxs_obs, self.vys_obs, self.omegas_obs = [], [], []
        self.lists = [self.vxs, self.vys, self.omegas, self.vxs_obs, self.vys_obs, self.omegas_obs]

        self.simulate()
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

            # Savefile must contain a valid directory
            if self.args.savefile:
                if not os.path.isdir(os.path.split(os.path.abspath(self.args.savefile))[0]):
                    print("Error: Argument provided to -s/--savefile is not a valid path!")
                    self.terminate()

                if os.path.splitext(self.args.savefile)[1] != ".txt":
                    print("Error: Argument provided to -s/--savefile must be a .txt file!")
                    self.terminate()

            if self.args.outfile:
                if not os.path.isdir(os.path.split(os.path.abspath(self.args.outfile))[0]):
                    print("Error: Argument provided to -o/--outfile is not a valid path!")
                    self.terminate()

                if os.path.splitext(self.args.outfile)[1] != ".cpp":
                    print("Error: Argument provided to -o/--outfile must be a .cpp file!")
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
                self.window.Q_textboxes[r][c].setText(str(self.kalmanFilter.Q[r,c]))
                self.window.Q_textboxes[r][c].textChanged.connect(partial(self.Q_array_change, r, c))

        for r, row in enumerate(self.kalmanFilter.R):
            for c, element in enumerate(row):
                self.window.R_textboxes[r][c].setText(str(self.kalmanFilter.R[r,c]))
                self.window.R_textboxes[r][c].textChanged.connect(partial(self.R_array_change, r, c))

    def Q_array_change(self, row, col):
        try:
            self.kalmanFilter.Q[row, col] = eval(self.window.Q_textboxes[row][col].text())
        except Exception:
            print("Make sure that element {},{} of the Q gains matrix is a valid number!".format(row, col))

    def R_array_change(self, row, col):
        try:
            self.kalmanFilter.R[row, col] = eval(self.window.R_textboxes[row][col].text())
        except Exception:
            print("Make sure that element {},{} of the R gains matrix is a valid number!".format(row, col))

    def update(self):
        t, x_hat, x = self.kalmanFilter.step(0.005, self.verbose, changeable_only=True)
        vx, vy, omega = x_hat
        vx_obs, vy_obs, omega_obs = x
        self.vxs.append(vx)
        self.vys.append(vy)
        self.omegas.append(omega)
        self.vxs_obs.append(vx_obs)
        self.vys_obs.append(vy_obs)
        self.omegas_obs.append(omega_obs)

    def set_filter_type(self, new_option):
        self.kalmanFilter.steady_state = (new_option == "Steady State")

    def simulate(self):
        for var_list in self.lists:
            var_list.clear()

        self.kalmanFilter.reset()

        for _ in self.ts:
            self.update()
        if not self.args.headless:
            self.window.set_plot_data(self.ts,
                                      self.vxs, self.vxs_obs,
                                      self.vys, self.vys_obs,
                                      self.omegas, self.omegas_obs)
            self.window.plot_fig.tight_layout()

    def save(self):
        if self.args.headless and self.args.savefile:
            file_name = self.args.savefile
        else:
            file_name = QFileDialog.getSaveFileName(self.window.central_widget, "Save Gains", '', "Text File (*.txt)")[0]
        if file_name:
            with open(file_name, 'w') as file:
                for key, pair in self.kalmanFilter.gains().items():
                    file.write(str(key) + ": " + str(pair) + "\n")

    def load(self):
        if self.args.headless and self.args.loadfile:
            file_name = self.args.loadfile
        else:
            file_name = QFileDialog.getOpenFileName(self.window.central_widget, "Import Gains", '', "Text File (*.txt)")[0]
        if file_name:
            with open(file_name, 'r') as file:
                gains_string = file.read()
                P, A, B, H, K, Q, R = [eval(gain.split(': ')[1]) for gain in gains_string.strip().split('\n')]
                self.kalmanFilter.set_gains(P, A, B, H, K, Q, R)
                if not self.args.headless:
                    self.populate_gains()

    def replace_gain(self, cpp_code, matrix_name, matrix):
        start = cpp_code.find("{} << ".format(matrix_name)) + 5
        end = start + cpp_code[start:].find(";")
        string = ""
        for row in matrix:
            string += " " * 9
            for element in row:
                string += str(element) + ", "
            string += "\n"
        string = string.strip(' ,\n')
        cpp_code = cpp_code[:start] + string + cpp_code[end:]
        return cpp_code

    def generate_code(self):
        if self.args.headless and self.args.outfile:
            file_name = self.args.outfile
        else:
            file_name = QFileDialog.getSaveFileName(self.window.central_widget, "Export C++ Code", '', "C++ (*.cpp)")[0]
        if file_name:
            with open(file_name, 'r+') as file:
                cpp_code = file.read()
                cpp_code = self.replace_gain(cpp_code, matrix_name="Q", matrix=self.kalmanFilter.Q.tolist())
                cpp_code = self.replace_gain(cpp_code, matrix_name="R", matrix=self.kalmanFilter.R.tolist())
                cpp_code = self.replace_gain(cpp_code, matrix_name="K", matrix=self.kalmanFilter.K.tolist())

                file.truncate(0)
                file.write(cpp_code)

if __name__ == '__main__':
    p = ArgumentParser()
    p.add_argument("-l", "--loadfile", dest="loadfile", help="File to load gains")
    p.add_argument("-hd", "--headless", action='store_true', help="Runs without gui")
    p.add_argument("-o", "--outfile", dest="outfile", help="File to export cpp code")
    p.add_argument("-r", "--regular", action='store_true', help="Runs Kalman filter without steady-state")
    p.add_argument("-s", "--savefile", dest="savefile", help="File to save gains")
    p.add_argument("-v", "--verbose", action='store_true', help="Prints Kalman Filter gains for every simulation step")
    args = p.parse_args()
    m = Main(args, kalmanFilter=KalmanFilter(x_hat_init=np.array([1, 0, 0]).reshape((3, 1)),
                                                init_covariance=0.1,
                                                steady_state=True),
                      verbose=False)
    sys.exit(m.exec_())