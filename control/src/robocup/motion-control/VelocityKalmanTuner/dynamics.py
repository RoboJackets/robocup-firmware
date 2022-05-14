from abc import ABC, abstractmethod


class Dynamics(ABC):
    """
    An abstract class which outlines the basic functionalities
    that an object simulating dynamics should implement
    """

    @abstractmethod
    def step(self, dt):
        pass

    @abstractmethod
    def get_state(self):
        pass

    @abstractmethod
    def get_measurements(self):
        pass
