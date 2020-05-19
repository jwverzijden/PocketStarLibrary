#include <SdFat.h>

#define APP_START_ADDRESS 0x10000

File file;

void initLoadingScreen() {
    pocketstar.drawRect(0, 0, 96, 64, true, BLACK_16b);
    pocketstar.setFont(pocketStar7pt);
    pocketstar.setFontColor(WHITE_16b, BLACK_16b);
    pocketstar.setCursor(48 - pocketstar.getPrintWidth("Loading...")/2, 22);
    pocketstar.print("Loading...");
    pocketstar.drawRect(15, 35, 66, 4, false, DARKGRAY_16b);
    pocketstar.setCursor(0, 0);
}

void showProgress(int state, float progress) {
    int width = (int) ((0.5 * (state - 1) + 0.5 * progress) * 64);
    pocketstar.drawRect(16, 36, width, 2, true, GREEN_16b);
}

void load(boolean g) {
    initLoadingScreen();
    
    char path[70];
    int pathLength = 0;
    if (g) gameDir.getName(path, 32);
    else appDir.getName(path, 32);
    pathLength = strlen(path);
    path[pathLength] = '/';
    if (g) gameDir.getName(path + pathLength + 1, 32);
    else appDir.getName(path + pathLength + 1, 32);
    strcpy(path + pathLength * 2 + 1, ".bin");
    if (g) file = games.open(path, FILE_READ);
    else file = apps.open(path, FILE_READ);
    if (!file) {
        pocketstar.setCursor(0, 0);
        pocketstar.setFontColor(RED_8b, BLACK_8b);
        pocketstar.print(".bin not found!");
        pocketstar.setFontColor(WHITE_8b, BLACK_8b);
        return;
    }
    
    uint32_t address = 64 * 1024;
    uint32_t PAGE_SIZE, PAGES, MAX_FLASH;
    uint32_t pageSizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};
    PAGE_SIZE = pageSizes[NVMCTRL->PARAM.bit.PSZ];
    PAGES = NVMCTRL->PARAM.bit.NVMP;
    MAX_FLASH = PAGE_SIZE * PAGES;
    uint8_t buffer[PAGE_SIZE];
    uint32_t erase_dst_addr = address; // starting address
    
    int i = 0;
    
    while (erase_dst_addr < MAX_FLASH) {
        // Execute "ER" Erase Row
        NVMCTRL->ADDR.reg = erase_dst_addr / 2;
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
        
        float progress = ((float) erase_dst_addr - address) / (MAX_FLASH - address);
        showProgress(1, progress);
        
        while (NVMCTRL->INTFLAG.bit.READY == 0)
            ;
            
        erase_dst_addr += PAGE_SIZE * 4; // Skip a ROW
    }
    
    // Write to flash
    uint32_t size = file.fileSize(); // PAGE_SIZE;
    uint32_t *dst_addr = (uint32_t *)(64 * 1024);
    uint8_t *ptr_data = buffer;
    
    // Set automatic page write
    NVMCTRL->CTRLB.bit.MANW = 0;
    // Set first address
    NVMCTRL->ADDR.reg = address / 2;
    // Do writes in pages
    while (size) {
        int amtBytes = file.read(buffer, PAGE_SIZE);
        
        if (size < 64) {
            for (int j = size; j < PAGE_SIZE; j++)
                buffer[j] = 0xFF;
        }
        // Execute "PBC" Page Buffer Clear
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
        while (NVMCTRL->INTFLAG.bit.READY == 0)
            ;

        // Fill page buffer
        uint32_t i;
        for (i = 0; i < (PAGE_SIZE / 4) && i < size; i++) {
            dst_addr[i] = ((uint32_t *)buffer)[i];
        }

        // Execute "WP" Write Page
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;

        float progress = 1 - ((float) size) / file.fileSize();
        showProgress(2, progress);

        while (NVMCTRL->INTFLAG.bit.READY == 0)
            ;

        // Advance to next page
        dst_addr += i;
        if (size > PAGE_SIZE)
            size -= PAGE_SIZE;
        else
            size = 0;
    }
    
    SPI.end();
    SPI1.end();
    
    __set_MSP(*(uint32_t *) APP_START_ADDRESS);
    SCB->VTOR = ((uint32_t) APP_START_ADDRESS & SCB_VTOR_TBLOFF_Msk);
    void (*appStart)(void);
    appStart = (void (*)(void))(unsigned *)(*(unsigned *)(APP_START_ADDRESS + 4));
    appStart();
}
