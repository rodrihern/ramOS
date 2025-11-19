CONTAINER_NAME="ramOS"

docker start $CONTAINER_NAME
docker exec -it $CONTAINER_NAME make -C /root
MAKE_EXIT_CODE=$?

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' 

# Check if make command was successful
if [[ $MAKE_EXIT_CODE -eq 0 ]]; then
    echo -e "${GREEN}Compilation successful!${NC}"
    echo -e "${GREEN}Run './run.sh' to start the kernel${NC}"
else
    echo -e "${RED}Compilation failed!${NC}"
fi