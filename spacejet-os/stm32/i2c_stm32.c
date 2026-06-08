/*
 * SpaceJet OS — I2C1 driver (STM32F405)
 * PB6 = SCL (AF4 = I2C1)
 * PB7 = SDA (AF4 = I2C1)
 * Speed: 100 kHz (standard mode), APB1 = 42 MHz
 */
#include "stm32f405.h"
#include "i2c_stm32.h"

#define I2C_TIMEOUT 100000U

static int wait_flag(volatile uint32_t *reg, uint32_t flag) {
    uint32_t t = I2C_TIMEOUT;
    while (!(*reg & flag)) if (!t--) return -1;
    return 0;
}

void i2c1_init(void) {
    RCC_AHB1ENR |= RCC_GPIOBEN;
    RCC_APB1ENR |= RCC_I2C1EN;

    /* PB6, PB7 — open-drain, AF4 */
    gpio_set_mode(GPIOB, 6, GPIO_AF);
    gpio_set_mode(GPIOB, 7, GPIO_AF);
    gpio_set_af  (GPIOB, 6, 4U);   /* AF4 = I2C1 */
    gpio_set_af  (GPIOB, 7, 4U);
    GPIOB->OTYPER |= BIT(6) | BIT(7);   /* open-drain */

    /* Reset I2C */
    I2C1->CR1 = BIT(15);   /* SWRST */
    I2C1->CR1 = 0;

    /* CR2: peripheral clock = 42 MHz */
    I2C1->CR2  = 42;
    /* CCR for 100 kHz: T_high = T_low = 5 µs; CCR = 5µs * 42MHz = 210 */
    I2C1->CCR  = 210;
    /* TRISE = (1µs * 42MHz) + 1 = 43 */
    I2C1->TRISE = 43;
    I2C1->CR1  = I2C_CR1_PE;
}

int i2c1_write(uint8_t addr7, const uint8_t *data, uint32_t len) {
    /* START */
    I2C1->CR1 |= I2C_CR1_START;
    if (wait_flag(&I2C1->SR1, I2C_SR1_SB)) return -1;
    /* Address + WRITE */
    I2C1->DR = (uint32_t)(addr7 << 1);
    if (wait_flag(&I2C1->SR1, I2C_SR1_ADDR)) return -1;
    (void)I2C1->SR2;   /* clear ADDR */
    /* Data */
    for (uint32_t i = 0; i < len; i++) {
        if (wait_flag(&I2C1->SR1, I2C_SR1_TXE)) return -1;
        I2C1->DR = data[i];
    }
    if (wait_flag(&I2C1->SR1, I2C_SR1_BTF)) return -1;
    /* STOP */
    I2C1->CR1 |= I2C_CR1_STOP;
    return 0;
}

int i2c1_read(uint8_t addr7, uint8_t *buf, uint32_t len) {
    I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_START;
    if (wait_flag(&I2C1->SR1, I2C_SR1_SB)) return -1;
    I2C1->DR = (uint32_t)((addr7 << 1) | 1U);
    if (wait_flag(&I2C1->SR1, I2C_SR1_ADDR)) return -1;
    (void)I2C1->SR2;
    for (uint32_t i = 0; i < len; i++) {
        if (i == len - 1) {
            I2C1->CR1 &= ~I2C_CR1_ACK;   /* NACK on last byte */
            I2C1->CR1 |= I2C_CR1_STOP;
        }
        if (wait_flag(&I2C1->SR1, I2C_SR1_RXNE)) return -1;
        buf[i] = (uint8_t)(I2C1->DR & 0xFF);
    }
    return 0;
}
