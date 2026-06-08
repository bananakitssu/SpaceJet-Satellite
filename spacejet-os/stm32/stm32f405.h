/*
 * SpaceJet OS — STM32F405 bare-metal register map
 * No HAL, no CMSIS — every register defined explicitly.
 */
#ifndef STM32F405_H
#define STM32F405_H
#include <stdint.h>

/* ── Bit helpers ── */
#define BIT(n)          (1UL << (n))
#define MMIO32(a)       (*(volatile uint32_t *)(uint32_t)(a))

/* ── RCC ── */
#define RCC_BASE        0x40023800U
#define RCC_CR          MMIO32(RCC_BASE + 0x00)
#define RCC_PLLCFGR     MMIO32(RCC_BASE + 0x04)
#define RCC_CFGR        MMIO32(RCC_BASE + 0x08)
#define RCC_AHB1ENR     MMIO32(RCC_BASE + 0x30)
#define RCC_APB1ENR     MMIO32(RCC_BASE + 0x40)
#define RCC_APB2ENR     MMIO32(RCC_BASE + 0x44)
/* AHB1ENR bits */
#define RCC_GPIOAEN     BIT(0)
#define RCC_GPIOBEN     BIT(1)
#define RCC_GPIOCEN     BIT(2)
#define RCC_DMA1EN      BIT(21)
#define RCC_DMA2EN      BIT(22)
/* APB1ENR bits */
#define RCC_TIM2EN      BIT(0)
#define RCC_I2C1EN      BIT(21)
#define RCC_SPI2EN      BIT(14)
#define RCC_USART2EN    BIT(17)
/* APB2ENR bits */
#define RCC_USART1EN    BIT(4)
#define RCC_SPI1EN      BIT(12)

/* ── GPIO (all ports share the same layout) ── */
typedef struct {
    volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR;
    volatile uint32_t IDR, ODR, BSRR, LCKR;
    volatile uint32_t AFR[2];
} GPIO_t;
#define GPIOA   ((GPIO_t *)0x40020000U)
#define GPIOB   ((GPIO_t *)0x40020400U)
#define GPIOC   ((GPIO_t *)0x40020800U)
/* MODER values (2 bits per pin) */
#define GPIO_IN     0U
#define GPIO_OUT    1U
#define GPIO_AF     2U
#define GPIO_ANALOG 3U
/* Speed */
#define GPIO_SPEED_HIGH  3U
/* Alternate functions */
#define AF5_SPI1   5U
#define AF6_SPI3   6U
#define AF7_USART  7U
#define AF9_I2C    4U   /* I2C1 = AF4 on STM32F405 */

static inline void gpio_set_mode(GPIO_t *port, int pin, uint32_t mode) {
    port->MODER = (port->MODER & ~(3U << (pin*2))) | (mode << (pin*2));
}
static inline void gpio_set_af(GPIO_t *port, int pin, uint32_t af) {
    int idx = pin / 8, shift = (pin % 8) * 4;
    port->AFR[idx] = (port->AFR[idx] & ~(0xFU << shift)) | (af << shift);
}
static inline void gpio_set(GPIO_t *port, int pin)   { port->BSRR = BIT(pin); }
static inline void gpio_clear(GPIO_t *port, int pin) { port->BSRR = BIT(pin+16); }
static inline int  gpio_read(GPIO_t *port, int pin)  { return (port->IDR >> pin) & 1; }

/* ── USART ── */
typedef struct {
    volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
} USART_t;
#define USART1  ((USART_t *)0x40011000U)
#define USART2  ((USART_t *)0x40004400U)
#define USART_SR_TXE   BIT(7)
#define USART_SR_TC    BIT(6)
#define USART_SR_RXNE  BIT(5)
#define USART_CR1_UE   BIT(13)
#define USART_CR1_TE   BIT(3)
#define USART_CR1_RE   BIT(2)

/* ── SPI ── */
typedef struct {
    volatile uint32_t CR1, CR2, SR, DR, CRCPR, RXCRCR, TXCRCR, I2SCFGR, I2SPR;
} SPI_t;
#define SPI1    ((SPI_t *)0x40013000U)
#define SPI2    ((SPI_t *)0x40003800U)
#define SPI_CR1_MSTR    BIT(2)
#define SPI_CR1_SSI     BIT(8)
#define SPI_CR1_SSM     BIT(9)
#define SPI_CR1_SPE     BIT(6)
#define SPI_SR_TXE      BIT(1)
#define SPI_SR_RXNE     BIT(0)
#define SPI_SR_BSY      BIT(7)

/* ── I2C ── */
typedef struct {
    volatile uint32_t CR1, CR2, OAR1, OAR2, DR, SR1, SR2, CCR, TRISE, FLTR;
} I2C_t;
#define I2C1    ((I2C_t *)0x40005400U)
#define I2C_CR1_PE      BIT(0)
#define I2C_CR1_START   BIT(8)
#define I2C_CR1_STOP    BIT(9)
#define I2C_CR1_ACK    BIT(10)
#define I2C_SR1_SB      BIT(0)
#define I2C_SR1_ADDR    BIT(1)
#define I2C_SR1_TXE     BIT(7)
#define I2C_SR1_RXNE    BIT(6)
#define I2C_SR1_BTF     BIT(2)

/* ── SysTick ── */
typedef struct {
    volatile uint32_t CTRL, LOAD, VAL, CALIB;
} SysTick_t;
#define SYSTICK ((SysTick_t *)0xE000E010U)
#define SYSTICK_CTRL_ENABLE     BIT(0)
#define SYSTICK_CTRL_TICKINT    BIT(1)
#define SYSTICK_CTRL_CLKSRC     BIT(2)   /* 1 = processor clock */

/* ── SCB ── */
#define SCB_ICSR    MMIO32(0xE000ED04U)
#define SCB_VTOR    MMIO32(0xE000ED08U)
#define SCB_SHPR2   MMIO32(0xE000ED1CU)   /* SVC priority */
#define SCB_SHPR3   MMIO32(0xE000ED20U)   /* SysTick[31:24] PendSV[23:16] */
#define ICSR_PENDSVSET  BIT(28)

/* ── NVIC ── */
#define NVIC_ISER0  MMIO32(0xE000E100U)
#define NVIC_ISER1  MMIO32(0xE000E104U)
#define NVIC_IPR(n) MMIO32(0xE000E400U + ((n)/4)*4)

/* ── Flash (needed for wait-states at high speed) ── */
#define FLASH_BASE  0x40023C00U
#define FLASH_ACR   MMIO32(FLASH_BASE + 0x00)
#define FLASH_LATENCY_5WS  5U   /* required at 168 MHz, 3.3 V */

#endif /* STM32F405_H */
