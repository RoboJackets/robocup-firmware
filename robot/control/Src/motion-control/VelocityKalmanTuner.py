import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import TextBox
from functools import partial
import matplotlib.gridspec as gridspec

'''
Credit: https://www.bzarg.com/p/how-a-kalman-filter-works-in-pictures/
https://en.wikipedia.org/wiki/Kalman_filter
https://www.mathworks.com/videos/understanding-kalman-filters-part-4-optimal-state-estimator-algorithm--1493129749201.html
'''


class Application(object):
    def __init__(self, kalmanFilter, verbose=False):
        self.fig = plt.figure('Velocity Kalman Filter Tuner')
        self.gs = self.fig.add_gridspec(3, 2)
        self.kalmanFilter = kalmanFilter
        self.verbose = verbose
        self.vx_ax = self.fig.add_subplot(self.gs[0, 0])
        self.vx_ax.set_title('X velocity over Time')
        self.vx_ax.set_xlabel('t')
        self.vx_ax.set_ylabel('vx')

        # Second Row: vy plot and R gains
        self.vy_ax = self.fig.add_subplot(self.gs[1, 0])
        self.vy_ax.set_title('Y velocity over Time')
        self.vy_ax.set_xlabel('t')
        self.vy_ax.set_ylabel('vy')

        # Third Row: Omega plot and buttons
        self.omega_ax = self.fig.add_subplot(self.gs[2, 0])
        self.omega_ax.set_title('Omega vs Time')
        self.omega_ax.set_xlabel('t')
        self.omega_ax.set_ylabel('\u03C9')  # omega character

        self.panel_ax = self.fig.add_subplot(self.gs[:, 1])
        self.panel_ax.axis('off')
        self.inner_gs = gridspec.GridSpecFromSubplotSpec(3, 1, self.panel_ax, height_ratios=[3, 5, 1])

        self.Q_ax = self.fig.add_subplot(self.inner_gs[0, 0])
        self.Q_ax.axis('off')
        self.Q_gs = gridspec.GridSpecFromSubplotSpec(3, 1, self.Q_ax, hspace=0)
        self.Q_axs = [self.fig.add_subplot(self.Q_gs[i, 0]) for i in range(0,3)]
        self.Q_textboxes = [TextBox(ax, "", str(self.kalmanFilter.Q.tolist()[i])) for i, ax in enumerate(self.Q_axs)]

        self.R_ax = self.fig.add_subplot(self.inner_gs[1, 0])
        self.R_ax.axis('off')
        self.R_gs = gridspec.GridSpecFromSubplotSpec(5, 1, self.R_ax, hspace=0)
        self.R_axs = [self.fig.add_subplot(self.R_gs[i, 0]) for i in range(0, 5 )]
        self.R_textboxes = [TextBox(ax, "", str(self.kalmanFilter.R.tolist()[i])) for i, ax in
                            enumerate(self.R_axs)]

        for i, textbox in enumerate(self.Q_textboxes):
            textbox.on_text_change(partial(self.Q_array_change, row=i))

        for i, textbox in enumerate(self.R_textboxes):
            textbox.on_text_change(partial(self.R_array_change, row=i))

        self.Q_axs[0].set_title("Q gains")
        self.R_axs[0].set_title("R gains")

        self.button_ax = self.fig.add_subplot(self.inner_gs[2, 0])
        self.button_ax.axis('off')
        self.button_gs = gridspec.GridSpecFromSubplotSpec(1, 2, self.button_ax)

        self.sim_ax = self.fig.add_subplot(self.button_gs[0, 0])
        self.simulate_btn = plt.Button(self.sim_ax, 'Simulate')
        self.simulate_btn.on_clicked(self.simulate)

        self.code_ax = self.fig.add_subplot(self.button_gs[0, 1])
        self.generate_code_btn = plt.Button(self.code_ax, 'Generate\nC++ Code')

        self.ts, self.vxs, self.vys, self.omegas = np.arange(0, 10, 0.01), [], [], []
        self.vxs_obs, self.vys_obs, self.omegas_obs = [], [], []
        self.lists = [self.vxs, self.vys, self.omegas, self.vxs_obs, self.vys_obs, self.omegas_obs]
        self.axs = [self.vx_ax, self.vy_ax, self.omega_ax]
        plt.show()

    def Q_array_change(self, _, row):
        try:
            self.kalmanFilter.Q[row] = eval(self.Q_textboxes[row].text)
            print(self.kalmanFilter.Q)
        except SyntaxError:
            print("Make sure each array element is separated by a comma and starts/ends with square brackets")

    def R_array_change(self, _, row):
        try:
            self.kalmanFilter.R[row] = eval(self.R_textboxes[row].text)
            print(self.kalmanFilter.R)
        except SyntaxError:
            print("Make sure each array element is separated by a comma and starts/ends with square brackets")

    def update(self):
        t, x_hat, x = self.kalmanFilter.step(0.01, self.verbose, changeable_only=True)
        vx, vy, omega = x_hat
        vx_obs, vy_obs, omega_obs = x
        self.vxs.append(vx)
        self.vys.append(vy)
        self.omegas.append(omega)
        self.vxs_obs.append(vx_obs)
        self.vys_obs.append(vy_obs)
        self.omegas_obs.append(omega_obs)

    def simulate(self, _):
        for ax in self.axs:
            if len(ax.lines) > 0:
                ax.lines.clear()

        for l in self.lists:
            l.clear()

        self.kalmanFilter.reset()

        for _ in self.ts:
            self.update()

        self.vx_ax.plot(self.ts, self.vxs, '-', c='blue')
        self.vy_ax.plot(self.ts, self.vys, '-', c='blue')
        self.omega_ax.plot(self.ts, self.omegas, '-', c='blue')

        self.vx_ax.plot(self.ts, self.vxs_obs, '-', c='green')
        self.vy_ax.plot(self.ts, self.vys_obs, '-', c='green')
        self.omega_ax.plot(self.ts, self.omegas_obs, '-', c='green')

        for ax in self.axs:
            ax.relim()
            ax.legend(['x_hat', 'x'], loc='upper right')

class KalmanFilter(object):
    def __init__(self, x_hat_init, init_covariance):
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
        self.x_hat = x_hat_init
        self.x = x_hat_init
        self.P = np.eye(self.num_states) * init_covariance
        self.A = np.eye(self.num_states)
        self.B = np.zeros((self.num_states, self.num_inputs))
        self.H = np.zeros((self.num_outputs, self.num_states))
        for i, angle in enumerate(self.wheel_angles):
            self.H[i - 1, :] = np.array([-np.sin(angle), np.cos(angle), self.wheel_dist])
        self.H[4, :] = np.array([0, 0, 1])
        self.H /= -self.wheel_radius

        self.Q = np.round(3.0 * self.process_noise * np.eye(self.num_states), 5)
        self.R = np.eye(self.num_outputs)
        self.R[:4, :4] *= self.encoder_noise
        self.R[4, 4] *= self.gyro_noise

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

        # A priori covariance estimate
        # P' = A * P_prev * A.T + Q
        self.P = self.A @ self.P @ self.A.T + self.Q

    def update_step(self):

        # Optimal Kalman Gain
        # K = P' * H.T * (H * P' * H.T + R)^-1
        K = self.P @ self.H.T @ np.linalg.inv(self.H @ self.P @ self.H.T + self.R)

        self.w = np.random.multivariate_normal((0, 0, 0), self.Q).reshape((3, 1))
        self.x = self.A @ self.x + self.B @ self.u + self.w

        # Observation
        self.v = np.random.multivariate_normal((0, 0, 0, 0, 0), self.R).reshape((5, 1))
        self.z = self.H @ self.x + self.v

        # Prefit
        # y = z - H * x_hat
        y = self.z - self.H @ self.x_hat

        # A posteriori state estimate
        # x_hat = x_hat' + K * (y - H * x_hat_est)
        self.x_hat += K @ y

        # A posteriori covariance estimate
        # P = (I - K * H) * P'
        self.P = (np.eye(3) - K @ self.H) @ self.P

    def reset(self):
        self.t = 0
        self.u = np.zeros((self.num_inputs, 1))
        self.z = np.zeros((self.num_outputs, 1))
        self.v = np.zeros((self.num_outputs, 1))
        self.w = np.zeros((self.num_states, 1))
        self.x_hat = self.x_hat_init
        self.x = self.x_hat_init
        self.P = np.eye(self.num_states) * self.init_covariance

    def vars(self, changeable_only=False):
        if changeable_only:
            return {
                't': self.t,
                'x_hat': self.x_hat,
                'P': self.P,
            }
        else:
            return {
                't': self.t,
                'x_hat': self.x_hat,
                'P': self.P,
                'A': self.A,
                'B': self.B,
                'H': self.H,
                'Q': self.Q,
                'R': self.R
            }


if __name__ == '__main__':
    app = Application(KalmanFilter(np.array([0, 0, 0]).reshape((3, 1)), 0.1), verbose=False)
