# logger

`logger` is a small reusable C++ logger package for the workspace.

## Usage

```cpp
#include <logger/Logger.h>

logger::Logger logger("my_package");
logger.info("Started");
logger.warn("Something looks off");
logger.error("Failed to continue");
```

## Consumer setup

Add the dependency in `package.xml`:

```xml
<depend>logger</depend>
```

Add it in `CMakeLists.txt`:

```cmake
find_package(logger REQUIRED)
ament_target_dependencies(your_target logger)
```

## Log files

Logs are written to CSV files under a timestamped run directory:

- `$HOME/.ros/log/logger/<YYYYMMDD_HHMMSS>/<logger_name>.csv`

Set `LOG_DIR` to override the root directory:

```bash
LOG_DIR=/tmp/legged_logs ros2 run logger logger_example
```

Set `LOG_RUN_ID` to force several processes to write under the same timestamp/run directory:

```bash
LOG_DIR=/workspace/leg_logs LOG_RUN_ID=20260618_101500 ros2 run logger logger_example
```

Each file uses this CSV header:

```csv
timestamp,level,message
```
# logger
