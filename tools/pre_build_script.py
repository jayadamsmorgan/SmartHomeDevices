import yaml
import os
Import("env")

print()
print("pre_built_script:")
print("-----------")

# include toolchain paths
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)

project_dir = env["PROJECT_DIR"]
with open(project_dir + "/device_settings.yaml") as settings_yaml_file:
    settings_yaml = yaml.safe_load(settings_yaml_file)
    required_settings = settings_yaml["Required"]
    print("Parsing YAML configuration file for " + settings_yaml["DEVICE_TYPE"] + "...")
    print()
    
    print("Required:")
    for setting in required_settings:
        if required_settings[setting] != None:
            print(setting + "=" + str(required_settings[setting]))
        else:
            print(setting)
    optional_settings = settings_yaml["Optional"]
    print()
    print("Optional:")
    for setting in optional_settings:
        if optional_settings[setting] != None:
            print(setting + "=" + str(optional_settings[setting]))
        else:
            print(setting)
    env.Append(CPPDEFINES=required_settings)
    env.Append(CPPDEFINES=optional_settings)
    env.Append(CPPDEFINES=[("DEVICE_TYPE", settings_yaml["DEVICE_TYPE"])])
    print()

print("-----------")
print("OK!")
print()

