from abc import ABC, abstractmethod


class Observer(ABC):
    """
    An abstract class which outlines the basic functionalities
    that a state-space observer should implement
    """

    @abstractmethod
    def predict(self):
        pass

    @abstractmethod
    def update(self):
        pass

    @abstractmethod
    def step(self):
        pass

    @abstractmethod
    def get_state_estimate(self):
        pass
