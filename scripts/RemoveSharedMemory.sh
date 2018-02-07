printf "Removing shared memory .. "

rm /tmp/qipc_sharedmemory* > /dev/null 2>&1
rm /tmp/qipc_systemsem* > /dev/null 2>&1

# Remove the single instance semaphore containing the RadeonDeveloperServiceCLI GUID in the filename.
rm /dev/shm/sem.D0939873-BA4B-4C4E-9729-D82DED85BC41 > /dev/null 2>&1
echo "done"
