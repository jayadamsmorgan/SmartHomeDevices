import yaml
Import("env")

print()
print("pre_built_script:")
print("-----------")

project_dir = env["PROJECT_DIR"]
with open(project_dir + "/device_settings.yaml") as settings_yaml_file:
    settings_yaml = yaml.safe_load(settings_yaml_file)
    required_settings = settings_yaml["Required"]
    optional_settings = settings_yaml["Optional"]
    env.Append(CPPDEFINES=required_settings)
    env.Append(CPPDEFINES=optional_settings)
    env.Append(CPPDEFINES=[("DEVICE_TYPE", settings_yaml["DEVICE_TYPE"])])

print("OK!")
print("-----------")
print()

