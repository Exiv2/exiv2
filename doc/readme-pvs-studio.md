# How to integrate PVS-Studio

We obtained a free license of PVS-Studio due to the open source nature of Exiv2.

## CMake integration

To check a project configured with CMake, such as Exiv2, we need to generate the JSON compilation database. 

```bash
# Under an already configured "buildXXX" directory
cd buildXXX
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ...

# Output file: compile_commands.json
```

The analysis starts with the following commands:

```bash
export PVS_LICENSE=~/.config/PVS-Studio/PVS-Studio.lic
pvs-studio-analyzer analyze -l $PVS_LICENSE -o pvsStudio.log -j8
plog-converter -a GA:1,2 -t tasklist pvsStudio.log -o pvsStudio.tasks
plog-converter -a GA:1,2 -t fullhtml pvsStudio.log -o pvsReportHtml
log-converter -a GA:1,2 -d V1042 -t fullhtml pvsStudio.log -o pvsReportHtml
```