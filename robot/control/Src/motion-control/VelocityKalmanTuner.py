import sys
import os
import re

import numpy as np
from PyQt5.QtWidgets import QApplication
from PyQt5.QtWidgets import (QGridLayout, QMainWindow, QWidget, QVBoxLayout, QGroupBox, QLabel, QLineEdit, QPushButton,
                             QHBoxLayout, QAction, QFileDialog, QComboBox)
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg, NavigationToolbar2QT
import matplotlib.pyplot as plt
from functools import partial
import scipy.linalg


'''
Credit: https://www.bzarg.com/p/how-a-kalman-filter-works-in-pictures/
https://en.wikipedia.org/wiki/Kalman_filter
https://www.mathworks.com/videos/understanding-kalman-filters-part-4-optimal-state-estimator-algorithm--1493129749201.html
'''


class UIMainWindow(QMainWindow):
    """Main Window for Application"""

    def __init__(self):
        super(UIMainWindow, self).__init__()
        self.setWindowTitle("Velocity Kalman Filter Tuner")
        self.resize(self.screen().size()*2/3)
        self.central_widget = QWidget(self)
        self.master_grid_layout = QGridLayout()

        self.plot_box = QGroupBox(self.central_widget)
        self.plot_box_layout = QVBoxLayout(self.plot_box)
        self.plot_fig = plt.figure()
        self.plot_canvas = FigureCanvasQTAgg(self.plot_fig)
        self.vx_ax = self.plot_fig.add_subplot(3, 1, 1)
        self.vy_ax = self.plot_fig.add_subplot(3, 1, 2)
        self.omega_ax = self.plot_fig.add_subplot(3, 1, 3)
        self.plot_axs = [self.vx_ax, self.vy_ax, self.omega_ax]

        self.vx_ax.set_title('X velocity over Time')
        self.vx_ax.set_xlabel('t')
        self.vx_ax.set_ylabel('vx')

        self.vy_ax.set_title('Y velocity over Time')
        self.vy_ax.set_xlabel('t')
        self.vy_ax.set_ylabel('vy')

        self.omega_ax.set_title('Omega vs Time')
        self.omega_ax.set_xlabel('t')
        self.omega_ax.set_ylabel('\u03C9')  # omega character

        for ax in self.plot_axs:
            ax.spines['top'].set_visible(False)
            ax.spines['right'].set_visible(False)

        self.plot_toolbar = NavigationToolbar2QT(self.plot_canvas, self)
        self.plot_box_layout.addWidget(self.plot_canvas)
        self.plot_box_layout.addWidget(self.plot_toolbar)

        self.right_box = QGroupBox(self.central_widget, title="Gains")
        self.right_box_layout = QVBoxLayout(self.right_box)

        self.settings_box = QGroupBox(self.central_widget)
        self.settings_box_layout = QVBoxLayout(self.settings_box)
        self.filter_type_box = QGroupBox(self.central_widget)
        self.filter_type_box_layout = QHBoxLayout(self.filter_type_box)
        self.filter_type_label = QLabel("Kalman Filter Type: ")
        self.filter_type_select = QComboBox(self.central_widget)
        self.filter_type_select.addItem("Regular")
        self.filter_type_select.addItem("Steady State")
        self.filter_type_box_layout.addWidget(self.filter_type_label)
        self.filter_type_box_layout.addWidget(self.filter_type_select)
        self.filter_type_box.setFlat(True)

        self.settings_box_layout.addWidget(self.filter_type_box)
        self.settings_box.setFlat(True)

        self.Q_box = QGroupBox(self.central_widget)
        self.Q_box_layout = QGridLayout(self.Q_box)
        self.Q_label = QLabel("Q Gains")
        self.Q_box_layout.addWidget(self.Q_label, 0, 0, 1, 1)
        self.Q_textboxes = []
        for i in range(1, 4):
            row = []
            for j in range(0, 3):
                textbox = QLineEdit(self.central_widget)
                row.append(textbox)
                self.Q_box_layout.addWidget(textbox, i, j, 1, 1)
            self.Q_textboxes.append(row)
        self.Q_box.setFlat(True)

        self.R_box = QGroupBox(self.central_widget)
        self.R_box_layout = QGridLayout(self.R_box)
        self.R_label = QLabel("R Gains")
        self.R_box_layout.addWidget(self.R_label, 0, 0, 1, 1)
        self.R_textboxes = []
        for i in range(1, 6):
            row = []
            for j in range(0, 5):
                textbox = QLineEdit(self.central_widget)
                row.append(textbox)
                self.R_box_layout.addWidget(textbox, i, j, 1, 1)
            self.R_textboxes.append(row)
        self.R_box.setFlat(True)

        self.buttons_box = QGroupBox(self.central_widget)
        self.buttons_box_layout = QHBoxLayout(self.buttons_box)
        self.sim_bttn = QPushButton("Simulate")
        self.gen_bttn = QPushButton("Generate C++ Code")
        self.buttons_box_layout.addWidget(self.sim_bttn)
        self.buttons_box_layout.addWidget(self.gen_bttn)
        self.buttons_box.setFlat(True)

        self.right_box_layout.addWidget(self.filter_type_box)
        self.right_box_layout.addWidget(self.Q_box)
        self.right_box_layout.addWidget(self.R_box)
        self.right_box_layout.addWidget(self.buttons_box)

        self.master_grid_layout.addWidget(self.plot_box, 0, 0, 1, 1)
        self.master_grid_layout.addWidget(self.right_box, 0, 1, 1, 1)
        self.master_grid_layout.setColumnStretch(0, 2)
        self.master_grid_layout.setColumnStretch(1, 1)

        self.setCentralWidget(self.central_widget)
        self.central_widget.setLayout(self.master_grid_layout)

        self.menu_bar = self.menuBar()
        self.save_action = QAction('Save Gains', self)
        self.save_action.setShortcut('Ctrl+S')
        self.load_action = QAction('Import Gains', self)
        self.load_action.setShortcut('Ctrl+I')
        self.file_menu = self.menu_bar.addMenu('File')
        self.file_menu.addAction(self.save_action)
        self.file_menu.addAction(self.load_action)


    def set_plot_data(self, ts, vxs, vxs_obs, vys, vys_obs, omegas, omegas_obs):
        for ax in self.plot_axs:
            ax.lines.clear()

        self.vx_ax.plot(ts, vxs, '-', c='tab:blue')
        self.vy_ax.plot(ts, vys, '-', c='tab:blue')
        self.omega_ax.plot(ts, omegas, '-', c='tab:blue')

        self.vx_ax.plot(ts, vxs_obs, '-', c='tab:green')
        self.vy_ax.plot(ts, vys_obs, '-', c='tab:green')
        self.omega_ax.plot(ts, omegas_obs, '-', c='tab:green')

        for ax in self.plot_axs:
            ax.relim()
            ax.legend(['x_hat', 'x'])

        self.plot_canvas.draw()

class Main(QApplication):
    def __init__(self, kalmanFilter, verbose=False):
        super(Main, self).__init__([])
        self.kalmanFilter = kalmanFilter
        self.verbose = verbose
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
        self.window.set_plot_data(self.ts,
                                  self.vxs, self.vxs_obs,
                                  self.vys, self.vys_obs,
                                  self.omegas, self.omegas_obs)
        self.window.plot_fig.tight_layout()

    def save(self):
        file_name = QFileDialog.getSaveFileName(self.window.central_widget, "Save Gains", '', "Text File (*.txt)")[0]
        if file_name:
            with open(file_name, 'w') as file:
                for key, pair in self.kalmanFilter.gains().items():
                    file.write(str(key) + ": " + str(pair) + "\n")

    def load(self):
        file_name = QFileDialog.getOpenFileName(self.window.central_widget, "Import Gains", '', "Text File (*.txt)")[0]
        if file_name:
            with open(file_name, 'r') as file:
                gains_string = file.read()
                P, A, B, H, K, Q, R = [eval(gain.split(': ')[1]) for gain in gains_string.strip().split('\n')]
                self.kalmanFilter.set_gains(P, A, B, H, K, Q, R)
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
        file_name = QFileDialog.getSaveFileName(self.window.central_widget, "Export C++ Code", '', "C++ (*.cpp)")[0]
        if file_name:
            with open(file_name, 'r+') as file:
                cpp_code = file.read()
                cpp_code = self.replace_gain(cpp_code, matrix_name="Q", matrix=self.kalmanFilter.Q.tolist())
                cpp_code = self.replace_gain(cpp_code, matrix_name="R", matrix=self.kalmanFilter.R.tolist())
                cpp_code = self.replace_gain(cpp_code, matrix_name="K", matrix=self.kalmanFilter.K.tolist())

                file.truncate(0)
                file.write(cpp_code)

class KalmanFilter(object):
    def __init__(self, x_hat_init, init_covariance, steady_state=False):
        """
        Initialize new Kalman Filter Object with
        (3 x 1) state : [v_x, v_y, omega] ([m/s, m/s, rad/s])
        (4 x 1) control input: [motor_1, motor_2, motor_3, motor_4] (% V)
        (5 x 1) output: [enc_1, enc_2, enc_3, enc_4, gyro_vel] (rad/s)

        :param x_hat: (3 x 1) state vector
        :param A: (3 x 3) matrix: transition/dynamics matrix
        :param B: (4 x 4) matrix: control matrix
        :param P: (3 x 3) matrix: covariance
        :param H: (5 x 3) matrix
        :param Q: (3 x 3) matrix
        :param R: (5 x 5) matrix:
        """

        self.steady_state = steady_state

        # Deg to Rad
        self.wheel_angles = np.deg2rad([180 - 30,
                                        180 + 39,
                                        360 - 39,
                                        0 + 30])
        self.wheel_dist = 0.0798576
        self.wheel_radius = 0.02786

        self.num_states = 3
        self.num_inputs = 4
        self.num_outputs = 5

        self.rng = np.random.default_rng()
        self.rng_5 = np.random.default_rng()

        self.process_noise = 0.05
        self.encoder_noise = 0.04
        self.gyro_noise = 0.005

        self.x_hat_init = x_hat_init
        self.init_covariance = init_covariance

        self.t = 0
        self.u = np.zeros((self.num_inputs, 1))
        self.z = np.zeros((self.num_outputs, 1))
        self.v = np.zeros((self.num_outputs, 1))
        self.w = np.zeros((self.num_states, 1))

        self.x_hat = self.x_hat_init
        self.x = self.x_hat_init

        self.A = np.eye(self.num_states)
        self.B = np.zeros((self.num_states, self.num_inputs))

        self.H = np.zeros((self.num_outputs, self.num_states))
        for i, angle in enumerate(self.wheel_angles):
            self.H[i - 1, :] = np.array([-np.sin(angle), np.cos(angle), self.wheel_dist])
        self.H[4, :] = np.array([0, 0, 1])
        self.H /= -self.wheel_radius

        self.Q = np.round(3.0 * self.process_noise * 0.0001 * np.eye(self.num_states), 10)
        self.R = np.eye(self.num_outputs)
        self.R[:4, :4] *= self.encoder_noise
        self.R[4, 4] *= self.gyro_noise

        if self.steady_state:
            self.P = scipy.linalg.solve_discrete_are(self.A.T, self.H.T, self.Q, self.R)
            self.K = self.P @ self.H.T @ np.linalg.inv(self.H @ self.P @ self.H.T + self.R)
        else:
            self.P = np.eye(self.num_states) * init_covariance
            self.K = np.zeros((self.num_states, self.num_outputs))

    def step(self, dt, verbose=False, changeable_only=False):
        self.t += dt
        self.prediction_step()
        self.update_step()
        if verbose:
            print("-" * 40)
            v = self.vars(changeable_only)
            for key in v:
                print(key, ":\n", v[key])
        return self.t, self.x_hat, self.x

    def generate_u(self):
        return np.zeros((self.num_inputs, 1))

    def prediction_step(self):
        # Control input, returns 0 for now
        self.u = self.generate_u()

        # A priori state estimate
        # x_hat' = A * x_hat_prev + B * u
        self.x_hat = self.A @ self.x_hat + self.B @ self.u

        if not self.steady_state:
            # A priori covariance estimate
            # P' = A * P_prev * A.T + Q
            self.P = self.A @ self.P @ self.A.T + self.Q

    def update_step(self):
        if not self.steady_state:
            # Optimal Kalman Gain
            # K = P' * H.T * (H * P' * H.T + R)^-1
            self.K = self.P @ self.H.T @ np.linalg.inv(self.H @ self.P @ self.H.T + self.R)

        # self.w = self.rng.multivariate_normal((0, 0, 0), self.Q, method='eigh').reshape((self.num_states,1))
        self.w = self.Q @ np.random.randn(self.num_states, 1)
        self.x = self.A @ self.x + self.B @ self.u + self.w

        # Observation
        # self.v = self.rng_5.multivariate_normal((0, 0, 0, 0, 0), self.R, method='eigh').reshape((self.num_outputs,1))
        self.v = self.R @ np.random.randn(self.num_outputs, 1)
        self.z = self.H @ self.x + self.v

        # Prefit
        # y = z - H * x_hat
        y = self.z - self.H @ self.x_hat

        # A posteriori state estimate
        # x_hat = x_hat' + K * (y - H * x_hat_est)
        self.x_hat += self.K @ y

        if not self.steady_state:
            # A posteriori covariance estimate
            # P = (I - K * H) * P'
            self.P = (np.eye(self.num_states) - self.K @ self.H) @ self.P

    def reset(self):
        self.t = 0
        self.u = np.zeros((self.num_inputs, 1))
        self.z = np.zeros((self.num_outputs, 1))
        self.v = np.zeros((self.num_outputs, 1))
        self.w = np.zeros((self.num_states, 1))
        self.x_hat = self.x_hat_init
        self.x = self.x_hat_init
        if self.steady_state:
            self.P = scipy.linalg.solve_discrete_are(self.A.T, self.H.T, self.Q, self.R)
            self.K = self.P @ self.H.T @ np.linalg.inv(self.H @ self.P @ self.H.T + self.R)
        else:
            self.P = np.eye(self.num_states) * self.init_covariance
            self.K = np.zeros((self.num_states, self.num_outputs))

    def vars(self, changeable_only=False):
        if changeable_only:
            return {
                't': self.t,
                'x': self.x,
                'x_hat': self.x_hat,
                'P': self.P,
                'K': self.K
            }
        else:
            return {
                't': self.t,
                'x': self.x,
                'x_hat': self.x_hat,
                'P': self.P,
                'A': self.A,
                'B': self.B,
                'H': self.H,
                'K': self.K,
                'Q': self.Q,
                'R': self.R
            }

    def gains(self):
        return {
            'P': self.P,
            'A': self.A,
            'B': self.B,
            'H': self.H,
            'K': self.K,
            'Q': self.Q,
            'R': self.R
        }

    def set_gains(self, P, A, B, H, K, Q, R):
        self.P = np.array(P).reshape((self.num_states, self.num_states))
        self.A = np.array(A).reshape((self.num_states, self.num_states))
        self.B = np.array(B).reshape((self.num_states, self.num_inputs))
        self.H = np.array(H).reshape((self.num_outputs, self.num_states))
        self.K = np.array(K).reshape((self.num_states, self.num_outputs))
        self.Q = np.array(Q).reshape((self.num_states, self.num_states))
        self.R = np.array(R).reshape((self.num_outputs, self.num_outputs))

if __name__ == '__main__':
    m = Main(kalmanFilter=KalmanFilter(x_hat_init=np.array([1, 0, 0]).reshape((3, 1)),
                                                init_covariance=0.1,
                                                steady_state=True),
                      verbose=False)
    sys.exit(m.exec_())