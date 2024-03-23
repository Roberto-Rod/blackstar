#!/bin/bash 

# Link to base server Ubuntu image: https://releases.ubuntu.com/focal/ubuntu-20.04.6-live-server-amd64.iso 

# Before proceeding to run this script please ensure that you perform the following steps:
# 1 - Connect embedded PC to a monitor and keyboard via the VersaLogic breakout cables.
# 2 - Connect embedded PC to an internet connection using the VersaLogic Ethernet breakout cable.
# 3 - Plug Ettus SDR into USB 3.0 port.

# The user will be asked to confirm the Windows script ran without errors before the rest of this script continues with updates and the required setup

echo "Starting Blackstar Ubuntu Image setup!"
echo "Performing apt updates..."
sudo apt update
sudo apt upgrade
sudo apt install openssh-server net-tools can-utils inotify-tools libuhd-dev uhd-host make git python3-pip unzip
sudo uhd_images_downloader
sudo cp /lib/uhd/utils/uhd-usrp.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
sudo pip install usb_resetter
sudo pip install pyserial
sudo apt-get install i2c-tools
sudo apt-get install python3-smbus
sudo pip3 install pycrc
sudo apt-get install python3-intelhex
sudo apt-get install setserial

# Before running the remaining of this script, we need to create some directories where we need to copy files into
# Upon creating these directories, we need to run the transfer_files.sh script on the windows host PC which will copy/transfer the required files via ssh
cd /home/blackstar-admin
sudo ln -sf /mnt/sed/admin-data/scripts ./scripts_sed
echo "Creating /home/blackstar-admin/scripts directory..."
cd /home/blackstar-admin/
sudo mkdir -p scripts
echo "Setting write permissions in /home/blackstar-admin/scripts directory..."
sudo chmod -R 777 /home/blackstar-admin/scripts
echo "Creating /home/blackstar-admin/calibration directory..."
sudo mkdir -p calibration
echo "Setting write permissions in /home/blackstar-admin/calibration directory..."
sudo chmod -R 777 /home/blackstar-admin/calibration
echo "Creating /etc/init.d/can_if directory..."
sudo mkdir -p /etc/init.d/can_if
echo "Setting write permissions in /etc/init.d/can_if directory..."
sudo chmod -R 777 /etc/init.d/can_if
echo "Creating VersaAPI directory..."
sudo mkdir -R /home/blackstar-admin/VersaAPI_Linux_64b_v1.8.2
echo "Setting write permissions in home/blackstar-admin/VersaAPI_Linux_64b_v1.8.2 directory..."
sudo chmod -R 777 /home/blackstar-admin/VersaAPI_Linux_64b_v1.8.2
echo "Finished creating required directories successfuly!"

# Request user to the transfer_files.sh file to transfer the required files
echo "Run the transfer_files.sh file on the PC connected to this Versalogic board via a ethernet, cable in a Git Bash terminal
Enter 'Y' or 'y' when this has been done and you are ready to proceed!"
echo "Enter 'N' or 'n' to abort!"

while true; do
read -s -n 1 key

case $key in
y|Y)
echo "You pressed $key. Continuing with Ubuntu image setup."
break
;;

n|N)
echo "You pressed $key. Operation canceled."
exit 0
;;
*)
echo "Invalid input. Please press 'y' or 'n'."
;;
esac
done

# Install the SDR 
echo "Installing USRP Hardware Driver Peripheral Report Utility..."
sudo uhd_usrp_probe

# Extract VersApi tar archive amnd install
cd /home/blackstar-admin/VersaAPI_Linux_64b_v1.8.2
echo "Extracting Versajet API tar archive..."
tar xvf VersaAPI_Linux_64b_v1.8.2.tar 
sudo rm VersaAPI_Linux_64b_v1.8.2.tar
echo "Installing VersaAPI..."
sudo chmod -R 777 .
sudo ./vl_install.sh EPU-4x62
sudo depmod -a

# Extract scripts.tar archive into /home/blackstar-admin/scripts and delete tar archive
echo "Extracting scripts.tar archive into /home/blackstar-admin/scripts and deleting tar archive..."
cd /home/blackstar-admin/scripts
tar xvf scripts.tar
sudo rm scripts.tar
cd /home/blackstar-admin/
touch calibration/pa_cal.csv
sudo chown -R blackstar-admin:blackstar-admin /home/blackstar-admin
chmod -R 700 /home/blackstar-admin

# Configure SED service
echo "Extracting SED scripts..."
cd /mnt/sed/admin-data/scripts/
tar xvf scripts_sed.tar
sudo rm scripts_sed.tar
echo "Configuring SED service..."
cd /etc/systemd/system
sudo ln -sf /home/blackstar-admin/scripts/sed.service ./
sudo systemctl daemon-reload
sudo systemctl enable sed.service

# Add the kernel boot args noexec=off  and libata.allow_tpm=1  following instructions here:
# https://askubuntu.com/questions/19486/how-do-i-add-a-kernel-boot-parameter
echo "Adding the kernel boot args..."
cd /etc/default
# Make a backup copy of grub before editing
sudo cp grub grub.bak
sudo sed -i 's/GRUB_CMDLINE_LINUX_DEFAULT="maybe-ubiquity"/GRUB_CMDLINE_LINUX_DEFAULT="noexec=off libata.allow_tpm=1"/g' grub
sudo update-grub

# Add CAN Network Up Script
echo "Setting up CAN network Up script..."    
# The file below was copied to this repo
# https://github.com/linux-can/can-misc/blob/master/etc/can_if 
cd /etc/init.d/can_if/
echo "Modifying the CAN network script..."
# Modify the following:
# CAN_IF="can0@1000000"
sudo sed -i 's/CAN_IF=""/CAN_IF="can0@1000000"/g' can_if.sh
# VCAN_IF=""
sudo sed -i 's/VCAN_IF="vcan0 vcan1 vcan2 vcan3"/VCAN_IF=""/g' can_if.sh
# PROBE=""
sudo sed -i 's/PROBE="vcan"/PROBE=""/g' can_if.sh
sudo update-rc.d can_if defaults

# Setup static IP configuration - Edit the file /etc/netplan/00-installer-config.yaml as superuser:
echo "Configuring neplan network..."
cd /etc/netplan
# Remove enp0s31f6 and its configuration so that it has no network
sudo sed -i '/enp0s31f6:/d' 00-installer-config.yaml
sudo sed -i '/dhcp4: true/d' 00-installer-config.yaml
sudo netplan --debug generate
sudo netplan -debug apply

# At this point we stop with the image creation and copy this image onto the other required SD cards.
# The next steps should be performed manually on each of the Blackstar modules

# # Add the regular 'blackstar' user and 'sed' group:
# sudo adduser blackstar
# # Password: <secret - see RH/JP: BlackStar Ubuntu Passwords>
# # Press enter for all other fields (don't enter Full Name etc.)
# sudo addgroup sed
# sudo usermod blackstar -a -G sed
# sudo usermod blackstar-admin -a -G sed
# cd /home
# sudo rm -rf blackstar
# sudo ln -sf /mnt/sed/user-data /home/blackstar

# # Open terminal as 'blackstar-admin':
# cd /mnt/sed
# mkdir -p admin-data/scripts
# mkdir admin-data/apps
# mkdir admin-data/ephem
# cd /mnt/sed/admin-data
# ln -sf /home/blackstar-admin/calibration ./
# sudo su blackstar
# cd /mnt/sed
# mkdir user-data
# chgrp sed user-data
# chmod 750 user-data
# cd user-data
# mkdir .ssh
# chmod 700 .ssh
# cd .ssh
# touch id_rsa
# touch id_rsa.pub
# touch authorized_keys
# chmod 600 id_rsa
# chmod 644 id_rsa.pub
# chmod 600 authorized_keys
# exit

# copy blackstar/scripts_sed into scripts
# copy BlackStarECM into apps
# copy gps-gen-realtime into apps
# copy ephemeris files into ephem
# copy keys into files above

# # apps - blackstar_ecm
# cd /home/blackstar-admin/mnt/sed/admin-data/apps/
# tar xvf blackstar_ecm.tar
# sudo rm blackstar_ecm.tar

# # TODO: apps - gps-gen-realtime

# # TODO: ephem

# # keys
# cd /home/blackstar/mnt/sed/user-data/.ssh
# tar xvf keys.tar
# sudo rm keys.tar
# # create backups, then rename the keys files
# mv -v id_rsa_blackstar-0001 id_rsa.bak
# mv -v id_rsa_blackstar-0001.pub id_rsa.pub.bak
# mv -v id_rsa_blackstar-0001 id_rsa
# mv -v id_rsa_blackstar-0001.pub id_rsa.pub


# # In terminal:
# chmod -R 700 /mnt/sed/admin-data