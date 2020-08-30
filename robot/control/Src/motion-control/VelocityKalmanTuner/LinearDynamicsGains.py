import numpy as np


class LinearDynamicsGains():
    """
    A class in charge of storing, returning, and verifying gains
    for LinearDynamics
    """
    def __init__(self, A, B, H, D, x_init):
        self.A = A
        self.B = B
        self.H = H
        self.D = D

        self.verify_A(x_init=x_init)
        self.verify_B()
        self.verify_H()

    def verify_A(self, x_init):
        if self.A == np.zeros(self.A.shape):
            raise ValueError("Dynamics matrix (A) should be non-zero!")
        if self.A.shape(0) != x_init.shape(0):
            raise ValueError("Dynamics matrix (A) should have the same number of rows as state vector (x_init)!")

    def verify_B(self):
        if self.A.shape(0) != self.B.shape(0):
            raise ValueError("Control matrix (B) should have the same number of rows as dynamics matrix (A)!")

    def verify_H(self):
        if self.H.shape(1) != self.A.shape(0):
            raise ValueError("Measurement matrix (H) should have the same number of rows as the columns of"
                             " dynamics matrix (A)!")
        if self.H == np.zeros(self.H.shape):
            raise ValueError("Measurement matrix (H) should be non-zero!")

    def verify_D(self):
        if self.D.shape(0) != self.H.shape(0):
            raise ValueError("Feedthrough matrix (D) should have same rows as measurement matrix (H)!")

    def dynamics_gains(self):
        return {
            'A': self.A,
            'B': self.B,
            'H': self.H,
            'D': self.D
        }