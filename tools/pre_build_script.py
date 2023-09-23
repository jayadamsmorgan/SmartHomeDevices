import json
Import("env")

print()
print("pre_built_script:")
print("-----------")

project_dir = env["PROJECT_DIR"]
with open(project_dir + "/device_settings.json") as settings_json_file:
    settings_json = json.load(settings_json_file)
    required_settings = settings_json["required"]
    optional_settings = settings_json["optional"]
    env.Append(CPPDEFINES=required_settings)
    env.Append(CPPDEFINES=optional_settings)
    env.Append(CPPDEFINES=[("DEVICE_TYPE", settings_json["DEVICE_TYPE"])])

print("OK!")
print("-----------")
print()

