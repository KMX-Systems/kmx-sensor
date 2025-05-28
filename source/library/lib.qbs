import qbs

StaticLibrary {
    Depends { name: "cpp" }
    consoleApplication: true
    cpp.cxxLanguageVersion: "c++23"
    cpp.enableRtti: false
    install: true
    name: "kmx-sensor-lib"
    cpp.includePaths: [
        "inc",
        "inc_dep"
    ]
    files: [
        "inc/kmx/sensor/data/base.hpp",
        "inc/kmx/sensor/data/humidity.hpp",
        "inc/kmx/sensor/data/light_intensity.hpp",
        "inc/kmx/sensor/data/temperature.hpp",
    ]
}
