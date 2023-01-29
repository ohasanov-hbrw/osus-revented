# config.py

def configure(env):
    env.Prepend(CFLAGS=["-std=c++17"])
    env.Prepend(CXXFLAGS=["-std=gnu++14"])

def can_build(env, platform):
    return True


def configure(env):
    pass
