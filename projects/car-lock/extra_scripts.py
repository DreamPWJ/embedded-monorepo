Import("env")

env.Append(CPPDEFINES=[
    ("NAME", env.StringifyMacro('Text is "潘维吉"')),
])