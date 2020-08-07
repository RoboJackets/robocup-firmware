import numpy as np
import scipy.linalg


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