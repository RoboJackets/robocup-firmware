import numpy as np


class KalmanFilterGains:
    def __init__(self, H, P, Q, R):
        self.H = H
        self.P = P
        self.Q = Q
        self.R = R

        self.verify_P()
        self.verify_Q()
        self.verify_R()
        self.update_K()
        self.verify_K()

    def verify_K(self):
        pass

    def verify_P(self):
        pass

    def verify_Q(self):
        pass

    def verify_R(self):
        pass

    def update_K(self):
        self.K = self.P @ self.H.T @ np.linalg.inv(self.H @ self.P @ self.H.T + self.R)

    def set_gains(self, H, P, Q, R, K=None):
        self.H = H
        self.P = P
        self.Q = Q
        self.R = R
        if K == None:
            self.update_K()

    def KF_gains(self):
        return {
            'K': self.K,
            'P': self.P,
            'Q': self.Q,
            'R': self.R
        }