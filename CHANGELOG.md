# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed

- `all="0"` in xmls is treated as no all attribute (#70)
- structures now can have barrier (#71) 

## [6.5.0] - 2023-07-11

- Updated Subdue skill now cancels an attack, if subdue reduces the attack to 0
- Update corrosive skill
- Added options `update-corrosive-protect-armor` and `no-update-corrosive-protect-armor` to toggle new behaviour (default: updated)

## [6.4.0] - 2023-04-14

- Updated `strict-db` to discard the database. This is now the default behavior for tuo. 
- Added `no-strict-db` to disable default 

## [6.3.1] - 2024-04-11

- Fixed Bravery On Play for 1D+ cards (#52)

## [6.3.0] - 2024-04-11

- Update Orbocalypse Raid and bges.txt

## [6.2.1] - 2023-04-11

- Changed poison to full damage.
- Fixed barrier protection on play (#45).
- Update Orbocalypse Raid and bges.txt

## [6.2.0] - 2023-04-07

- Update counter skill
- Added options `update-counter-without-damage` and `no-update-counter-without-damage` to toggle new behaviour (default: updated)
- Update poison skill
- Added options `update-poison-after-attacked` and `no-update-poison-after-attacked` to toggle new behaviour (default: updated)

## [6.1.0] - 2023-04-04

- Added changes of versions [4.5.0-4.8.0] to database+ml tuo version [5.0.0-6.0.0]

## [4.8.0] - 2023-04-04

- Update subdue skill
- Added options `update-subdue-before-attack` and `no-update-subdue-before-attack` to toggle new behaviour (default: updated)
- Changed order of operations on an attack to: 
    Flying
    Subdue
    Attack-Damage
    OA-Skills
    Counter
    Leech
    Drain
    Berserk
    Mark

## [4.7.0] - 2023-04-02

- Update leech skill
- Added options `update-leech-increase-max-hp` and `no-update-leech-increase-max-hp` to toggle new behaviour (default: updated)

## [4.6.0] - 2023-04-02

- Update mimic skill
- Added options `update-dont-evade-mimic-selection` and `no-update-dont-evade-mimic-selection` to toggle new behaviour (default: updated)

## [4.5.0] - 2023-04-02

- Update barrier skill
- Added options `update-barrier-each-turn` and `no-update-barrier-each-turn` to toggle new behaviour (default: updated)

## [6.0.0] - 2023-04-02

- Add machine learning in data/*.pmml
- To train you own model checkout ml/README.md

## [5.0.0] - 2023-03-25

- Add database in data/database.yml
