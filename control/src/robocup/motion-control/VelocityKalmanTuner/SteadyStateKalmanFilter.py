import numpy as np
import scipy.linalg
import scipy.integrate as integrate
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
        self.x = x_hat_init

        A_k, B_k, H_k, D_k, _ = scipy.signal.cont2discrete(
            system=(A, B, H, D), dt=self.dt
        )
        self.dynamics = LinearDynamics(A_k, B_k, H_k, D_k, x_init=x_hat_init, dt=dt)
        self.num_states = self.dynamics.gains.A_k.shape[1]
        self.num_inputs = self.dynamics.gains.B_k.shape[1]
        self.num_outputs = self.dynamics.gains.H_k.shape[0]

        # https://en.wikipedia.org/wiki/Discretization
        # Q_k = np.round(integrate.quad_vec(lambda tau: scipy.linalg.expm(A*tau) @ Q @ scipy.linalg.expm(A.T*tau), 0, dt)[0], 10)
        # R_k = R / self.dt
        Q_k = Q
        R_k = R

        P_ss = scipy.linalg.solve_discrete_are(
            self.dynamics.gains.A_k.T, self.dynamics.gains.H_k.T, Q_k, R_k
        )

        self.gains = KalmanFilterGains(H_k=H_k, P=P_ss, Q_k=Q_k, R_k=R_k)
        self.step_response = False

    def predict(self):
        self.t += self.dt
        # Control input, returns 0 for now
        u = self.generate_u()

        # A priori state estimate
        # x_hat' = A * x_hat_prev + B * u
        self.dynamics.step(x=self.x_hat, u=u, Q=self.gains.Q_k, R=self.gains.R_k)
        self.x_hat = self.dynamics.get_state()

    def generate_u(self):
        return np.zeros((self.dynamics.gains.B_k.shape[1], 1))

    def update(self):
        # Prefit
        # y = z - H * x_hat
        y = 0
        if not self.step_response:
            y = self.dynamics.get_measurements() - self.gains.H_k @ self.x_hat
        else:
            a = 1
            self.x = np.heaviside((self.t - a) * np.ones((self.num_states, 1)), a)
            v = self.gains.R_k @ np.random.randn(self.num_outputs, 1)
            y = self.gains.H_k @ self.x - self.gains.H_k @ self.x_hat
        # A posteriori state estimate
        # x_hat = x_hat' + K * (y - H * x_hat_est)
        self.x_hat += self.gains.K @ y

        # Postfit
        # y = z - H * x_hat
        y = self.dynamics.get_measurements() - self.gains.H_k @ self.x_hat

    def get_state_estimate(self):
        return self.x_hat

    def step(self):
        self.predict()
        self.update()
        if not self.step_response:
            return self.t, self.get_state_estimate(), self.dynamics.get_state()
        else:
            return self.t, self.get_state_estimate(), self.x

    def set_gains(self, P_ss, A_k, B_k, H_k, D_k, K_k, Q_k, R_k):

        self.dynamics.gains.set_gains(A_k, B_k, H_k, D_k)
        self.gains.set_gains(H_k, P_ss, Q_k, R_k)

    def reset(self):
        self.t = 0
        if not self.step_response:
            self.x_hat = self.x_hat_init
        else:
            self.x_hat = np.zeros((self.x_hat.shape[0], 1))
        self.dynamics.reset()

    def get_gains(self):
        return {
            "dt": self.dt,
            "P_k": self.gains.P,
            "A_k": self.dynamics.gains.A_k,
            "B_k": self.dynamics.gains.B_k,
            "H_k": self.dynamics.gains.H_k,
            "D_k": self.dynamics.gains.D_k,
            "K_k": self.gains.K,
            "Q_k": self.gains.Q_k,
            "R_k": self.gains.R_k,
        }
