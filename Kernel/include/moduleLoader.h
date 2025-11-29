#ifndef MODULELOADER_H
#define MODULELOADER_H

void loadModules(void *payloadStart, void **targetModuleAddress, uint64_t * moduleSizes);

#endif