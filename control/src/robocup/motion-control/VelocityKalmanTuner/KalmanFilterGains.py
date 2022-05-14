import numpy as np


class KalmanFilterGains:
    def __init__(self, H_k, P, Q_k, R_k):
        self.H_k = H_k
        self.P = P
        self.Q_k = Q_k
        self.R_k = R_k

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
        self.K = (
            self.P
            @ self.H_k.T
            @ np.linalg.inv(self.H_k @ self.P @ self.H_k.T + self.R_k)
        )

    def set_gains(self, H_k, P, Q_k, R_k, K=None):
        self.H = H_k
        self.P = P
        self.Q = Q_k
        self.R = R_k
        if K is None:
            self.update_K()

    def KF_gains(self):
        return {"K": self.K, "P": self.P, "Q": self.Q_k, "R": self.R_k}
