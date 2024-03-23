#!/bin/bash 
echo "Transferring the required test scripts to the remote Ubuntu machine..."

# # copy test scripts to a temporary directory 
# scp -r C:/workspace/blackstar/hw_tests/blk_star_platform_tests/tests.tar blackstar-admin@192.168.6.2:/home/blackstar-admin/temp/test

# # Copy raspberry pi test scripts
# cd C:/workspace/blackstar/hw_tests/test_equipment
# # tar cvf test_equipment.tar *
# echo "Copying test equipment scripts to blackstar-test@192.168.6.99:/home/blackstar-test/"
# scp -r C:/workspace/blackstar/hw_tests/test_equipment blackstar-test@192.168.6.99:/home/blackstar-test/

# # Copy raspberry pi can files
# cd C:/workspace/blackstar/hw_tests/test_equipment
# # tar cvf test_equipment.tar *
# echo "Copying can files to blackstar-test@192.168.6.99:/home/blackstar-test/"
# scp -r C:/workspace/blackstar/hw_tests/blk_star_pc_tests/can_files blackstar-test@192.168.6.99:/home/blackstar-test/can_dump_logs