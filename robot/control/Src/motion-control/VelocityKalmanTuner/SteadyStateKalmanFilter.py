import numpy as np
import scipy.linalg
from observer import Observer
from KalmanFilterGains import KalmanFilterGains
from LinearDynamics import LinearDynamics


class SS_KF(Observer):
    """
    Steady State Discrete Kalman Filter.
    Estimates state using a precalculated convergent Kalman Gain
    Let n be the number of states, m the number of inputs, and o the number of outputs
    :param A (n x n) : Continuous-time state transition matrix
    :param B (n x m) : Continuous-time control matrix
    :param C (o x n) : Continuous-time observation matrix
    :param D (o x m) : Continuous-time feed-forward matrix
    :param Q (n x n) : Continuous-time covariance process noise matrix
    :param R (o x o) : Continuous-time covariance observation noise matrix
    :param dt: Time interval for system discretization
    """

    def __init__(self, x_hat_init, A, B, H, D, Q, R, dt=0.01):
        self.t = 0
        self.dt = dt

        self.x_hat_init = x_hat_init
        self.x_hat = x_hat_init

        A_k, B_k, H_k, D_k, _ = scipy.signal.cont2discrete(system=(A, B, H, D), dt=self.dt)

        self.dynamics = LinearDynamics(A_k, B_k, H_k, D_k, x_init=x_hat_init, dt=dt)
        self.num_states = self.dynamics.gains.A_k.shape[1]
        self.num_inputs = self.dynamics.gains.B_k.shape[1]
        self.num_outputs = self.dynamics.gains.H_k.shape[0]

        P_ss = scipy.linalg.solve_discrete_are(self.dynamics.gains.A_k.T, self.dynamics.gains.H_k.T, Q, R)

        self.gains = KalmanFilterGains(H=H, P=P_ss, Q=Q, R=R)

    def predict(self):
        # Control input, returns 0 for now
        u = self.generate_u()

        # A priori state estimate
        # x_hat' = A * x_hat_prev + B * u
        self.dynamics.step(dt=self.dt, x=self.x_hat, u=u, Q=self.gains.Q, R=self.gains.R)
        self.x_hat = self.dynamics.get_state()

    def generate_u(self):
        return np.zeros((self.dynamics.gains.B_k.shape[1], 1))

    def update(self):
        # Prefit
        # y = z - H * x_hat
        y = self.dynamics.get_measurements() - self.gains.H @ self.x_hat

        # A posteriori state estimate
        # x_hat = x_hat' + K * (y - H * x_hat_est)
        self.x_hat += self.gains.K @ y

        # Postfit
        # y = z - H * x_hat
        y = self.dynamics.get_measurements() - self.gains.H @ self.x_hat

    def get_state_estimate(self):
        return self.x_hat

    def step(self):
        self.predict()
        self.update()
        return self.t, self.get_state_estimate(), self.dynamics.get_state()

    def set_gains(self, P_ss, A_k, B_k, H_k, D_k, K_k, Q_k, R_k):

        self.dynamics.gains.set_gains(A_k, B_k, H_k, D_k)
        self.gains.set_gains(H_k, P_ss, Q_k, R_k)

    def reset(self):
        self.t = 0
        self.x_hat = self.x_hat_init

    def get_gains(self):
        return {
            'dt': self.dt,
            'P_k': self.gains.P,
            'A_k': self.dynamics.gains.A_k,
            'B_k': self.dynamics.gains.B_k,
            'H_k': self.dynamics.gains.H_k,
            'D_k': self.dynamics.gains.D_k,
            'K_k': self.gains.K,
            'Q_k': self.gains.Q,
            'R_k': self.gains.R
        }