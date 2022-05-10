from dynamics import Dynamics
from LinearDynamicsGains import LinearDynamicsGains
import scipy.signal
import numpy as np


class LinearDynamics(Dynamics):
    """
    A class which steps through a dynamic linear system of the form
    x' = Ax + Bu
    y = Hx
    """

    def __init__(self, A_k, B_k, H_k, D_k, x_init, dt=0.01):
        """
        Initializes constants for the system
        :param A: Dynamics matrix, converts current state to next state
        :param B: Control matrix, converts control input into state change
        :param H: Measurement matrix, converts state into measurements
        :param x_init: Initial state
        :param dt: Species the interval between timesteps. Helps save computation by computing discretized system
        only once if dt is the same for all timesteps.
        If 'None', then the system will be computed every time the function "step" is called
        """
        self.gains = LinearDynamicsGains(
            A_k=A_k, B_k=B_k, H_k=H_k, D_k=D_k, x_init=x_init
        )
        self.x_init = x_init
        self.reset()
        self.dt = dt

    def process_noise(self, Q):
        return np.random.multivariate_normal(mean=np.zeros_like(self.x), cov=Q)

    def measurement_noise(self, R):
        return np.random.multivariate_normal(mean=np.zeros_like(self.y), cov=R)

    def step(self, x, u, Q, R):
        """
        Based on currently tracked state, update state according to discrete
        dynamics
        """
        self.x = self.gains.A_k @ x + self.gains.B_k @ u + self.process_noise(Q=Q)
        self.y = self.gains.H_k @ x + self.gains.D_k @ u + self.measurement_noise(R=R)

    def get_state(self):
        return self.x

    def get_measurements(self):
        return self.y

    def reset(self):
        self.x = self.x_init
        self.y = self.gains.H_k @ self.x
