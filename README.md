# Smart Hospital - Patient & Resource Allocation System

A console-based hospital management program written in C (Code::Blocks project). It registers patients, allocates ward beds, calculates bills automatically and produces summary reports through a simple text menu.

## Author
Ravindu Lakshan

## Features
- **Patient registration**: title (Mr./Mrs./Miss), name, age, emergency level, specialty and admission status.
- **Bed allocation**: an admitted patient is given the first free bed in the chosen ward. A message is shown if the ward is full.
- **Automatic billing**: consultation fee + emergency surcharge + ward stay cost, minus an age subsidy, printed as a formatted bill.
- **Bed status**: total, occupied and available beds for every ward.
- **Priority list**: patients sorted so the most critical appear first.
- **Performance report**: patients per urgency level, total revenue, total discounts, bed occupancy % per ward and the highest paying patient.
- **File storage**: bed occupancy is saved on exit and loaded on start; every bill is appended to a records file.

## Menu
1. Register Patient
2. Check Bed Status
3. View Patients by Priority
4. Generate Performance Report
5. Exit

## Billing Rules
| Item | Rule |
|------|------|
| Emergency surcharge | Normal: 0% - Urgent: 20% - Critical: 50% of the consultation fee |
| Ward stay cost | Days admitted x daily bed rate of the ward |
| Age subsidy | 15% of the gross total if age is below 5 or above 65 |
| Final amount | Gross total - subsidy |

### Specialties
| Specialty | Fee (LKR) | Time per patient (mins) |
|-----------|-----------|-------------------------|
| General Practice (OPD) | 1500 | 15 |
| Paediatrics | 2500 | 20 |
| Cardiology | 4500 | 30 |
| Neurology | 5000 | 30 |

### Wards
| Ward | Beds | Daily rate (LKR) |
|------|------|------------------|
| General Ward | 20 | 3000 |
| Paediatric Ward | 10 | 6000 |
| Surgical Ward | 10 | 12000 |
| ICU | 5 | 25000 |

The estimated waiting time is the number of patients already queued for the specialty multiplied by that specialty's time per patient.

## Files
| File | Purpose |
|------|---------|
| `main.c` | Full source code |
| `Smart Hospital.cbp` | Code::Blocks project file |
| `beds_status.txt` | Created at runtime, stores bed occupancy |
| `patient_records.txt` | Created at runtime, stores one line per bill |

## How to Run
1. Open `Smart Hospital.cbp` in Code::Blocks.
2. Press **F9** (Build and Run).
3. Choose options from the menu in the console window.

## Limitations
- Up to 100 patients per session.
- Only bed occupancy is remembered between runs. The patient list and reports cover the current session only.