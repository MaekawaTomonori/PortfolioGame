@echo off
cd ..
git pull
git submodule update --init --recursive
echo All Updated
call GeneratedProjects.bat
pause