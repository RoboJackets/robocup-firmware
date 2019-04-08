from conans import ConanFile, CMake

class MtrainConan(ConanFile):
    name = "mTrain"
    version = "0.1.0"
    url = "https://github.com/RoboJackets/mtrain-firmware"
    description = "mTrain description"
    # settings = "os", "compiler", "build_type", "arch"
    settings = {"build_type": None, "compiler": None, "arch": ["armv7hf"]}
    generators = "cmake"
    exports_sources = ['CMakeLists.txt', 'API*', 'BSP*', 'external*']

    def build(self):
        cmake = CMake(self)
        # cmake.verbose = True
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy(pattern='*.so*', dst='lib', src='lib')

    def package(self):
        self.copy(pattern='*.h', dst='API/c/Inc', src='API/c/Inc')
        self.copy(pattern='*.h', dst='API/cpp/Inc', src='API/cpp/Inc')