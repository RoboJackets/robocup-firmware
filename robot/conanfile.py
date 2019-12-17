from conans import ConanFile, CMake

class RoboCupFirmwareConan(ConanFile):
    name = "RoboCupFirmware"
    version = "0.1.0"
    url = "https://github.com/RoboJackets/robocup-firmware"
    description = "firmware description"
    settings = {"build_type": None, "compiler": None, "arch": ["armv7hf"]}
    generators = "cmake"
    exports_sources = ['CMakeLists.txt', 'control*', 'lib*']

    requires = (('mTrain/[1.0.0]@robojackets/stable'),
                ('eigen/[3.3.7]@conan/stable'))


    def build(self):
        cmake = CMake(self)
        # cmake.verbose = True
        cmake.configure(args=['--no-warn-unused-cli'])
        cmake.build()

    def package(self):
        self.copy(pattern='control.*', dst='', src='bin')
