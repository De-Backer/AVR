import subprocess

revision = (
    subprocess.check_output(["git", "rev-parse", "--short=8", "--verify", "HEAD"])
    .strip()
    .decode("utf-8")
)
print("'-D GIT_COMMIT_SHA=\"%s\"'" % revision)