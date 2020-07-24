import numpy as np

'''
Credit: https://www.bzarg.com/p/how-a-kalman-filter-works-in-pictures/
https://en.wikipedia.org/wiki/Kalman_filter
https://www.mathworks.com/videos/understanding-kalman-filters-part-4-optimal-state-estimator-algorithm--1493129749201.html
'''
class KalmanFilter(object):
    def __init__(self, x_hat_init, init_covariance):
        """
        Initialize new Kalman Filter Object with
        (3 x 1) state : [x, y, omega] ([m/s, m/s, rad/s])
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
        self.wheel_angles = np.deg2rad([60, 60, 60, 60])

        self.num_states = 3
        self.num_inputs = 4
        self.num_outputs = 5

        self.process_noise = 0.05
        self.encoder_noise = 0.04
        self.gyro_noise = 0.005

        self.t = 0
        self.x_hat = x_hat_init
        self.P = np.eye(self.num_states) * init_covariance
        self.A = np.eye(self.num_states)
        self.B = np.zeros((self.num_states, self.num_inputs))
        self.H = np.zeros((self.num_outputs, self.num_states))
        for i, angle in enumerate(self.wheel_angles):
            self.H[i-1, :] = np.array([-np.sin(angle), np.cos(angle), 1])
        self.H[4,:] = np.array([0, 0, 1])

        self.Q = 3.0 * self.process_noise * np.eye(self.num_states)
        self.R = np.eye(self.num_outputs)
        self.R[:4, :4] *= self.encoder_noise
        self.R[4, 4] *= self.gyro_noise

    def step(self, dt, verbose=False, changeable_only=False):
        self.t += dt
        self.prediction_step()
        self.update_step()
        if verbose:
            print("-" * 40)
            v = filter.vars(changeable_only)
            for key in v:
                print(key, ":\n", v[key])
        return self.x_hat

    def generate_u(self):
        return np.zeros((self.num_inputs, 1))

    def generate_z(self):
        return np.array([1, 1, 1, 1, 1]).reshape((5, 1))

    def prediction_step(self):
        # Control input, returns 0 for now
        u = self.generate_u()

        # A priori state estimate
        # x_hat' = A * x_hat_prev + B * u
        self.x_hat = self.A @ self.x_hat + self.B @ u

        # A priori covariance estimate
        # P' = A * P_prev * A.T + Q
        self.P = self.A @ self.P @ self.A.T + self.Q

    def update_step(self):

        # Optimal Kalman Gain
        # K = P' * H.T * (H * P' * H.T + R)^-1
        K = self.P @ self.H.T @ np.linalg.inv(self.H @ self.P @ self.H.T + self.R)

        # Observation
        z = self.generate_z()

        # Prefit
        # y = z - H * x_hat
        y = z - self.H @ self.x_hat

        # A posteriori state estimate
        # x_hat = x_hat' + K * (y - H * x_hat_est)
        self.x_hat += K @ (y - self.H @ self.x_hat)

        # A posteriori covariance estimate
        # P = (I - K * H) * P'
        self.P = (np.eye(3) - K @ self.H) @ self.P

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
    filter = KalmanFilter(np.ones((3, 1)), 0.1)
    for i in range(100):
        filter.step(0.005, True, True)