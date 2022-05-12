import numpy as np


class LinearDynamicsGains:
    """
    A class in charge of storing, returning, and verifying gains
    for LinearDynamics
    """

    def __init__(self, A_k, B_k, H_k, D_k, x_init):
        self.x_init = x_init
        self.A_k = A_k
        self.B_k = B_k
        self.H_k = H_k
        self.D_k = D_k

        self.set_gains(A_k, B_k, H_k, D_k)

    def verify_A(self, x_init):
        if (self.A_k == np.zeros(self.A_k.shape)).all():
            raise ValueError("Dynamics matrix (A) should be non-zero!")
        if self.A_k.shape[0] != x_init.shape[0]:
            raise ValueError(
                "Dynamics matrix (A) should have the same number of rows as state vector (x_init)!"
            )

    def verify_B(self):
        if self.A_k.shape[0] != self.B_k.shape[0]:
            raise ValueError(
                "Control matrix (B) should have the same number of rows as dynamics matrix (A)!"
            )

    def verify_H(self):
        if self.H_k.shape[1] != self.A_k.shape[0]:
            raise ValueError(
                "Measurement matrix (H) should have the same number of rows as the columns of"
                " dynamics matrix (A)!"
            )
        if (self.H_k == np.zeros(self.H_k.shape)).all():
            raise ValueError("Measurement matrix (H) should be non-zero!")

    def verify_D(self):
        if self.D_k.shape[0] != self.H_k.shape[0]:
            raise ValueError(
                "Feedthrough matrix (D) should have same rows as measurement matrix (H)!"
            )

    def verify_observability(self):
        n = self.A_k.shape[1]
        q = self.H_k.shape[0]
        array = np.zeros((n * q, n))
        for i in range(0, n):
            array[i * q : (i + 1) * q, :] = self.H_k @ np.linalg.matrix_power(
                self.A_k, i
            )
        if np.linalg.matrix_rank(np.array(array)) != self.A_k.shape[1]:
            pass
            # TODO(Thomas):
            # Figure out why system is computed as non-observable when imported once
            # raise ValueError("System is not observable!")

    def verify_controllability(self):
        n = self.A_k.shape[1]
        q = self.B_k.shape[1]
        C = np.zeros((n, n * q))
        for i in range(0, n):
            C[:, i * q : (i + 1) * q] = np.linalg.matrix_power(self.A_k, i) @ self.B_k
        if np.linalg.matrix_rank(np.array(C)) != self.A_k.shape[1]:
            raise ValueError("System is not controllable!")

    def verify_system(self):
        self.verify_A(x_init=self.x_init)
        self.verify_B()
        self.verify_H()
        self.verify_D()
        self.verify_observability()
        # self.verify_controllability()

    def set_gains(self, A_k, B_k, H_k, D_k):
        self.A_k = A_k
        self.B_k = B_k
        self.H_k = H_k
        self.D_k = D_k

        self.verify_system()

    def dynamics_gains(self):
        return {"A_k": self.A_k, "B_k": self.B_k, "H_k": self.H_k, "D_k": self.D_k}
