from platformio.managers.platform import PlatformBase


class LibretuyaPlatform(PlatformBase):
    def configure_default_packages(self, options, targets):
        framework = options.get("pioframework")[0]
        if framework.startswith("realtek-ambz"):
            self.packages["toolchain-gccarmnoneeabi"]["version"] = "~1.40804.0"
        return super().configure_default_packages(options, targets)
