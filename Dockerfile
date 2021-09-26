# Use phusion/baseimage if problems arise
FROM ubuntu:20.04
MAINTAINER Collin Avidano collin.avidano@gmail.com

# Setup apt to be happy with no console input
ENV DEBIAN_FRONTEND noninteractive

# setup apt tools and other goodies we want
# left nano and htop because someone is going to ask why they cant use nano if this gets used as a development image
RUN apt-get update --fix-missing && apt-get -y install udev git vim nano software-properties-common sudo apt-utils wget curl htop iputils-ping less bsdmainutils screen && apt-get clean

# set up user
# Replace 1000 with your user / group id
RUN export uid=1000 gid=1000 && \
    mkdir -p /home/developer && \
    echo "developer:x:${uid}:${gid}:Developer,,,:/home/developer:/bin/bash" >> /etc/passwd && \
    echo "developer:x:${uid}:" >> /etc/group && \
    echo "developer ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/developer && \
    chmod 0440 /etc/sudoers.d/developer && \
    chown ${uid}:${gid} -R /home/developer && mkdir -p /etc/udev/rules.d/

USER developer
ENV HOME /home/developer

COPY . $HOME/robocup-firmware
WORKDIR $HOME/robocup-firmware

RUN sudo ./util/ubuntu-setup

RUN sudo chown -R developer:developer $HOME/
