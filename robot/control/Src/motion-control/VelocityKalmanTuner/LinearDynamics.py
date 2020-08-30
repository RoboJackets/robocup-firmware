from dynamics import Dynamics
from LinearDynamicsGains import LinearDynamicsGains
import scipy
class LinearDynamics(Dynamics):
    """
    A class which steps through a dynamic linear system of the form
    x' = Ax + Bu
    y = Hx
    """

    def __init__(self, A, B, H, D, x_init):
        """
        Initializes constants for the system
        :param A: Dynamics matrix, converts current state to next state
        :param B: Control matrix, converts control input into state change
        :param H: Measurement matrix, converts state into measurements
        :param x_init: Initial state
        """
        self.gains = LinearDynamicsGains(A=A, B=B, H=H, D=D, x_init=x_init)
        self.x = x_init
        self.y = self.gains.H @ self.x

    def step(self, dt):
        """
        Based on currently tracked state, update state according to discrete
        dynamics
        :param dt:
        :return:
        """
        A_k, B_k, H_k, _, _ = scipy.signal.cont2discrete(system=
                                                         (self.gains.A, self.gains.B, self.gains.H, self.gains.D),
                                                         dt=dt)
        self.x = A_k @ self.x + B_k @ self.u
        self.y = H_k @ self.x

    def get_state(self):
        return self.x

    def get_measurements(self):
        return self.y
