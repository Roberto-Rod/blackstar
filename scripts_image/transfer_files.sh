#!/bin/bash 
echo "Transferring the required files to the remote Ubuntu machine..."

# Copy the CAN configuration files
echo "Copying the CAN network setup script into /etc/init.d directory..."
scp "K:/Engineering/Project Files/Project_BlackStar/Reference/can_if/can_if.sh" blackstar-admin@192.168.6.2:/etc/init.d/can_if/can_if.sh

# Copy root scripts
echo "Updating git repository..."
cd C:/workspace/blackstar
git fetch --all
git pull --all
echo "Creating scripts.tar archive from scripts_root"
cd C:/workspace/blackstar/scripts_root
tar cvf scripts.tar *
echo "Copying the blackstar scripts tar archive into /home/blackstar-admin/scripts..."
scp -r "C:/workspace/blackstar/scripts_root/scripts.tar" blackstar-admin@192.168.6.2:/home/blackstar-admin/scripts/

# copy Versajet API files
cd C:/workspace/blackstar/applications/VersaAPI_Linux_64b_v1.8.2
tar cvf VersaAPI_Linux_64b_v1.8.2.tar *
scp -r "C:/workspace/blackstar/applications/VersaAPI_Linux_64b_v1.8.2/VersaAPI_Linux_64b_v1.8.2.tar" blackstar-admin@192.168.6.2:home/blackstar-admin/VersaAPI_Linux_64b_v1.8.2

# copy blackstar/scripts_sed into scripts
cd C:/workspace/blackstar/scripts_sed
tar cvf scripts_sed.tar *
echo "Copying the blackstar scripts_sed tar archive into /mnt/sed/admin-data/scripts..."
scp -r "C:/workspace/blackstar/scripts_sed/scripts_sed.tar" blackstar-admin@192.168.6.2:/mnt/sed/admin-data/scripts/

# # Request user to run the remainder of the create_ubuntu_image.sh script
# echo "Run the remainder of the create_ubuntu_image.sh script
# Enter 'Y' or 'y' when this has been done and you are ready to proceed!"
# echo "Enter 'N' or 'n' to abort!"

# while true; do
# read -s -n 1 key

# case $key in
# y|Y)
# echo "You pressed $key. Continuing with Ubuntu image setup."
# break
# ;;

# n|N)
# echo "You pressed $key. Operation canceled."
# exit 0
# ;;
# *)
# echo "Invalid input. Please press 'y' or 'n'."
# ;;
# esac
# done

# Run the next steps manually after the SED drive has been setup and mounted properly onm each of the Blackstar modules

# # copy blackstar/scripts_sed into scripts
# cd C:/workspace/blackstar/scripts_sed
# tar cvf scripts_sed.tar *
# echo "Copying the blackstar scripts_sed tar archive into /mnt/sed/admin-data/scripts..."
# scp -r "C:/workspace/blackstar/scripts_sed/scripts_sed.tar" blackstar-admin@192.168.6.2:/mnt/sed/admin-data/scripts/

# # copy BlackStarECM into apps
# cd C:/workspace/blackstar_ecm
# tar cvf blackstar_ecm.tar *
# echo "Copying blackstar_ecm tar archive into blackstar-admin@192.168.6.2:/mnt/sed/admin-data/apps..."
# scp -r "C:/workspace/blackstar_ecm/blackstar_ecm.tar" blackstar-admin@192.168.6.2:/mnt/sed/admin-data/apps/

# # # copy gps-gen-realtime into apps
# # # TODO


# # copy ephemeris files into ephem
# cd "C:/Users/rrodrigues/OneDrive - Kirintec UK/Projects/Blackstar/Ubuntu Image Generation/ubx-ephemerides"
# tar cvf ephemerides.tar *
# echo "Copying ubx-ephemerides tar archive into blackstar-admin@192.168.6.2:/mnt/sed/admin-data/ephem..."
# scp -r "C:/Users/rrodrigues/OneDrive - Kirintec UK/Projects/Blackstar/Ubuntu Image Generation/ubx-ephemerides/ephemerides.tar" blackstar-admin@192.168.6.2:/mnt/sed/admin-data/ephem


# # copy keys into files above
# cd C:/workspace/blackstar/keys
# tar cvf keys.tar *
# scp "C:/workspace/blackstar/keys/keys.tar" blackstar@192.168.6.2:/mnt/sed/user-data/.ssh