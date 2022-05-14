import numpy as np


class RobotModel:
    """
    Calculates and returns matrices for our current system model
    """

    def __init__(self):
        # wheel_angles, wheel_dist, & wheel_radius can all be found in robot_model.hpp
        self.wheel_angles = np.deg2rad([180 - 30, 180 + 39, 360 - 39, 0 + 30])
        self.wheel_dist = 0.0798576
        self.wheel_radius = 0.02786

        # Noises
        self.init_covariance = 0.003
        self.process_noise = 0.05
        self.encoder_noise = 0.04
        self.gyro_noise = 0.005

        # Dimensions:
        self.num_states = 3
        self.num_inputs = 4
        self.num_outputs = 5

    def get_A_matrix(self):
        A = np.zeros((self.num_states, self.num_states))
        return A

    def get_B_matrix(self):
        B = np.zeros((self.num_states, self.num_inputs))
        return B

    def get_H_matrix(self):
        H = np.zeros((self.num_outputs, self.num_states))
        for i, angle in enumerate(self.wheel_angles):
            H[i, :] = np.array([-np.sin(angle), np.cos(angle), self.wheel_dist])

        H /= -self.wheel_radius
        H[4, :] = np.array([0, 0, 1])
        return H

    def get_D_matrix(self):
        D = np.zeros((self.num_outputs, self.num_inputs))
        return D

    def get_P_init_matrix(self):
        P = np.eye(self.num_states) * self.init_covariance
        return P

    def get_Q_matrix(self):
        Q = np.round(3.0 * self.process_noise * 0.0001 * np.eye(self.num_states), 10)
        return Q

    def get_R_matrix(self):
        R = np.eye(self.num_outputs)
        R[:4, :4] *= self.encoder_noise
        R[4, 4] *= self.gyro_noise
        return R

    def generate_model(self):
        return {
            "A": self.get_A_matrix(),
            "B": self.get_B_matrix(),
            "H": self.get_H_matrix(),
            "D": self.get_D_matrix(),
            "P": self.get_P_init_matrix(),
            "Q": self.get_Q_matrix(),
            "R": self.get_R_matrix(),
        }
