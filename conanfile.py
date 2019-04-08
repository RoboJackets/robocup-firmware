from conans import ConanFile, CMake

class MtrainConan(ConanFile):
    name = "mTrain"
    version = "0.1.0"
    url = "https://github.com/RoboJackets/mtrain-firmware"
    description = "mTrain description"
    # settings = "os", "compiler", "build_type", "arch"
    settings = {"os": None, "build_type": None, "compiler": None, "arch": ["armv7e-m/hf"]}
    generators = "cmake"

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        cmake.build()
