import os
import shutil

# Удаляем проблемную папку, если она существует
problem_path = "components/idf_test"
if os.path.exists(problem_path):
    print(f"Removing problematic folder: {problem_path}")
    shutil.rmtree(problem_path)