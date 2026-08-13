These two files are **GitHub project management/documentation files**. They are not device-driver source files.

### 1. `CONTRIBUTING.md`

This explains **how someone should contribute to your project**.

For your BeagleBone Black project, it can contain:

```text
CONTRIBUTING.md
│
├── Project Contribution Guidelines
├── Development Environment
├── Coding Standards
├── Driver Development Rules
├── Device Tree Rules
├── Commit Message Format
├── Testing Requirements
├── Pull Request Process
└── Bug Reporting
```

Example:

````markdown
# Contributing

Thank you for contributing to the BeagleBone Black Linux Device Driver
Development project.

## Development Environment

- BeagleBone Black
- Linux host PC
- GCC
- Make
- Linux kernel headers
- Device Tree Compiler (dtc)
- Git

## Driver Development Rules

Each driver should contain:

- Driver source code
- Makefile
- README.md
- Device Tree configuration if required
- User-space test application
- Test procedure

## Coding Standards

Follow Linux kernel coding style.

Use meaningful names and avoid unnecessary global variables.

## Testing

Every driver must be tested on the BeagleBone Black.

Required validation:

```bash
dmesg
lsmod
modinfo <driver>
````

Functional testing must be documented in the driver's README.

## Commit Format

Use:

```text
driver: add GPIO interrupt driver
driver: add I2C sensor driver
dts: add SPI device configuration
test: add GPIO functional test
docs: update UART driver documentation
```

## Pull Requests

A pull request should contain:

1. Description of the change
2. Hardware used
3. Kernel version
4. Device Tree changes
5. Test procedure
6. Test results
7. Relevant `dmesg` output

````

---

### 2. `CHANGELOG.md`

This records **what changed in each version of your project**.

For example:

```markdown
# Changelog

All important changes to this project are documented here.

## [v1.0.0] - 2026-08-13

### Added

- Initial project structure
- Character driver framework
- GPIO LED driver
- GPIO button driver
- GPIO interrupt driver
- Device Tree examples
- Driver build scripts
- Basic user-space test applications

### Documentation

- Added hardware setup documentation
- Added Linux driver architecture documentation
- Added Device Tree documentation
- Added driver debugging guide

---

## [v0.9.0]

### Added

- Initial BeagleBone Black development environment
- Kernel build configuration
- Driver development framework

---

## [Unreleased]

### Planned

- I2C sensor driver
- SPI driver
- UART driver
- ADC/IIO driver
- PWM driver
- CAN driver
- RTC driver
- USB driver
- DMA driver
- Ethernet testing
- ALSA/ASoC driver experiments
- Watchdog driver
- Power management
- Stress testing
````

### In your project structure

So these files sit at the **root level**:

```text
beaglebone-black-linux-device-drivers/
│
├── README.md              ← What the project is
├── LICENSE                ← License
├── CONTRIBUTING.md        ← How to contribute
├── CHANGELOG.md           ← Project history
│
├── docs/
├── hardware/
├── device-tree/
├── drivers/
├── user-space/
├── scripts/
├── tests/
├── kernel/
└── tools/
```

**Simple difference:**

| File              | Purpose                                 |
| ----------------- | --------------------------------------- |
| `README.md`       | What is this project and how to use it? |
| `CONTRIBUTING.md` | How can another developer contribute?   |
| `CHANGELOG.md`    | What changed between versions?          |
| `LICENSE`         | What are the usage/legal terms?         |

For your portfolio, I would definitely keep all four. `CONTRIBUTING.md` can be relatively short, while `CHANGELOG.md` becomes more useful as you add each driver.

