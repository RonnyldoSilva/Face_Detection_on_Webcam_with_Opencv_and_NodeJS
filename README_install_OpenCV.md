# How to install OpenCV 3.2
 
### Update packages

```
sudo apt-get update
sudo apt-get upgrade
```

### Install Ubuntu libraries

```
sudo apt-get remove x264 libx264-dev

sudo apt-get install build-essential checkinstall cmake pkg-config yasm
sudo apt-get install git gfortran
sudo apt-get install libjpeg8-dev libjasper-dev libpng12-dev
 
# If you are using Ubuntu 14.04
sudo apt-get install libtiff4-dev
# If you are using Ubuntu 16.04
sudo apt-get install libtiff5-dev
 
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libdc1394-22-dev
sudo apt-get install libxine2-dev libv4l-dev
sudo apt-get install libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
sudo apt-get install qt5-default libgtk2.0-dev libtbb-dev
sudo apt-get install libatlas-base-dev
sudo apt-get install libfaac-dev libmp3lame-dev libtheora-dev
sudo apt-get install libvorbis-dev libxvidcore-dev
sudo apt-get install libopencore-amrnb-dev libopencore-amrwb-dev
sudo apt-get install x264 v4l-utils
```

### Install Python libraries

```
sudo apt-get install python-pip
sudo apt-get install python3-pip

# Install virtual environment
sudo pip2 install virtualenv virtualenvwrapper
sudo pip3 install virtualenv virtualenvwrapper
echo "# Virtual Environment Wrapper"  >> ~/.bashrc
echo "source /usr/local/bin/virtualenvwrapper.sh" >> ~/.bashrc
source ~/.bashrc
  
############ For Python 2 ############
# create virtual environment
mkvirtualenv facecourse-py2 -p python2
workon facecourse-py2
  
# now install python libraries within this virtual environment
pip install numpy scipy matplotlib scikit-image scikit-learn ipython
  
# quit virtual environment
deactivate
######################################
  
############ For Python 3 ############
# create virtual environment
mkvirtualenv facecourse-py3 -p python3
workon facecourse-py3
  
# now install python libraries within this virtual environment
pip install numpy scipy matplotlib scikit-image scikit-learn ipython
  
# quit virtual environment
deactivate
######################################
```

### Configure compiling environment
```
sudo pip install cython
```

### Upgrade g++5
```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get upgrade
```

### Install Opencv

```
cd ~

sudo apt-get update
sudo apt-get upgrade

sudo apt-get install libxvidcore-dev libx264-dev
sudo apt-get install libgtk-3-dev

sudo apt-get install libatlas-base-dev gfortran

wget -O opencv.zip https://github.com/Itseez/opencv/archive/3.2.0.zip 
unzip opencv.zip

wget -O opencv_contrib.zip https://github.com/Itseez/opencv_contrib/archive/3.2.0.zip
unzip opencv_contrib.zip

wget https://bootstrap.pypa.io/get-pip.py

sudo python get-pip.py
sudo pip install virtualenv virtualenvwrapper
sudo rm -rf ~/get-pip.py ~/.cache/pip
```

### Update ~/.bashrc

```
gedit ~/.bashrc
```

Add in the end of file:
```
# virtualenv and virtualenvwrapper
export WORKON_HOME=$HOME/.virtualenvs
source /usr/local/bin/virtualenvwrapper.sh
```

```
source ~/.bashrc
mkvirtualenv cv -p python3
sudo pip install numpy
```

##### Update /usr/local/include/tesseract/unichar.h 

```
sudo gedit /usr/local/include/tesseract/unichar.h	
```

Se necessário, coloque “std::” before “string” in the line 164.

### Compile and Install

```
cd opencv-3.2.0/

mkdir build

cd build
```

```
cmake -D CMAKE_BUILD_TYPE=RELEASE       -D CMAKE_INSTALL_PREFIX=/usr/local       -D INSTALL_C_EXAMPLES=ON       -D INSTALL_PYTHON_EXAMPLES=ON       -D WITH_TBB=ON       -D WITH_V4L=ON       -D WITH_QT=ON       -D WITH_OPENGL=ON       -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-3.2.0/modules       -D BUILD_EXAMPLES=ON .. 
```

```
# find out number of CPU cores in your machine
nproc

# substitute '4' by output of nproc
make -j4 --ignore-errors
sudo make install
sudo sh -c 'echo "/usr/local/lib" >> /etc/ld.so.conf.d/opencv.conf'
sudo ldconfig
```

### Create symlink in virtual environment

```
############ For Python 2 ############
cd ~/.virtualenvs/facecourse-py2/lib/python2.7/site-packages
ln -s /usr/local/lib/python2.7/dist-packages/cv2.so cv2.so
  
############ For Python 3 ############
cd ~/.virtualenvs/facecourse-py3/lib/python3.6/site-packages
ln -s /usr/local/lib/python3.6/dist-packages/cv2.cpython-36m-x86_64-linux-gnu.so cv2.so
```

### Test the OpenCV

Download this repository, then unzip `RedEyeRemover.zip`. Go to inside it.
```
git clone https://github.com/RonnyldoSilva/Install-and-Training-Tesseract-4-OCR-Opencv.git
```

Test C++ code:
```
# compile
# There are backticks ( ` ) around pkg-config command not single quotes
g++ -std=c++11 removeRedEyes.cpp `pkg-config --libs --cflags opencv` -o removeRedEyes
# run
./removeRedEyes
```

Test Python code:
```
python removeRedEyes.py
```