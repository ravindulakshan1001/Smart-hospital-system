   /*
    * Smart Hospital & Resource Allocation System
    * Author  : Ravindu Lakshan
    * Purpose : Registers patients, allocates ward beds, calculates bills
    *           and generates hospital performance reports.
    */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_PATIENTS 100
#define NUM_SPECIALITY 4
#define NUM_WARD 4
#define MAX_BED_PER_WARD 20
#define NAME_LENGTH 50


#define URGENCY_NORMAL 1
#define URGENCY_URGENT 2
#define URGENCY_CRITICAL 3

#define SURCHARGE_URGENT 20
#define SURCHARGE_CRITICAL 50


#define AGE_YOUNG 5
#define AGE_ADULT 65
#define SUBSIDY_PERCENT 15

char specialtyNames[NUM_SPECIALITY][NAME_LENGTH] = {"General Practice(OPD)", "Paediatrics", "Cardiology", "Neurology"};
float specialtyConslationFee[NUM_SPECIALITY] = {1500.00, 2500.00, 4500.00, 5000.00};
int specialtyConslationTime[NUM_SPECIALITY] = {15, 20, 30, 30};
int specialtyPatientsCapacity[NUM_SPECIALITY] = {30, 20, 12, 10};

char wardName[NUM_WARD][NAME_LENGTH] = {"General Ward", "Paediatric Ward", "Surgical Ward", "ICU"};
float wardDailyBedRate[NUM_WARD] = {3000.00, 6000.00, 12000.00, 25000.00};
int wardTotalBedCapacity[NUM_WARD] = {20, 10, 10, 5};

int bedOccupancy[NUM_WARD][MAX_BED_PER_WARD] = {0};
int patientCount = 0;
int specialtyQueueCount[NUM_SPECIALITY] = {0};

float patientFinalBill[MAX_PATIENTS];
float patientDiscount[MAX_PATIENTS];

int getValidInt(int min, int max, char *prompt);
int allocateBed(int wardId, int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[]);
void checkBedStatus(int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[]);

float calculateWaitingTime(int specialityId, int queueCount[], int specialityTime[]);
float calculateEmergencySurcharge(int urgencyLevel, float baseFee);
float calculateTotalWardStayCost(int daysAdmitted, int wardId, float wardRate[]);
float calculateGrossTotalBill(float baseConsultanFee, float emegencySurcharge, float totalWardCost);
float calculateSubsidyDiscount(int age, float grossTotal);
float calculateFinalAmountPayable(float grossTotal, float discount);
void printBill(int patientId, char name[], int age, int specialtyId, int wardId, int isAdmitted,
               int urgencyLevel, int daysAdmitted, float baseFee, float surcharge, float wardCost,
               float grossTotal, float discount, float finalAmount, float waitingTime, int bedNum);

void saveBedStatus(int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[]);
void loadBedStatus(int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[]);
void appendPatientRecord(int patientId, char name[], float finalAmount);

void registerPatients(char patientName[][NAME_LENGTH], int patientAge[], int patientEmergencyLevel[],
                      int specialtyId[], int isAdmitted[], int wardId[], int daysAdmitted[],
                      int specialtyQueueCount[], float patientFinalBill[], float patientDiscount[], int *count);

void sortAndDisplayByPriority(char patientName[][NAME_LENGTH], int patientEmergencyLevel[], int count);
void generateSummaryReport(int patientEmergencyLevel[], float patientFinalBill[], float patientDiscount[],
                            char patientName[][NAME_LENGTH], int count,
                            int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[]);

   // Reads an integer from the user and repeats until it is between min and max                            
int getValidInt(int min, int max, char *word)
{
    int value;
    int valid;

    do {
        printf("%-20s: ", word);
        valid = scanf("%d", &value);

        if(valid != 1){
            printf("  >> Invalid input. Please enter a number.\n");
            while(getchar() != '\n');
            value = min - 1;
        }
        else if(value < min || value > max){
            printf("  >> Invalid input. Please enter a number between %d and %d.\n", min, max);
        }
    } while(value < min || value > max);

    return value;
}

// Finds the first free bed in a ward, marks it occupied and returns the bed number (-1 if full)
int allocateBed(int wardId, int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[])
{
    int bedNum;
    for(bedNum = 0; bedNum < wardCapacity[wardId-1]; bedNum++){
        if(bedOccupancy[wardId-1][bedNum] == 0){
            bedOccupancy[wardId-1][bedNum] = 1;
            return bedNum + 1;
        }
    }
    return -1;
}

// Displays total, occupied and available beds for every ward
void checkBedStatus(int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[])
{
    int ward, bed, occupiedCount;

    printf("\n============================================================\n");
    printf("                   BED OCCUPANCY STATUS\n");
    printf("============================================================\n");

    for(ward = 0; ward < NUM_WARD; ward++){
        occupiedCount = 0;
        for(bed = 0; bed < wardCapacity[ward]; bed++){
            if(bedOccupancy[ward][bed] == 1){
                occupiedCount++;
            }
        }
        printf("\n%s\n", wardName[ward]);
        printf("------------------------------------------------------------\n");
        printf("%-20s: %d\n", "Total Beds", wardCapacity[ward]);
        printf("%-20s: %d\n", "Occupied", occupiedCount);
        printf("%-20s: %d\n", "Available", wardCapacity[ward] - occupiedCount);
    }
    printf("\n============================================================\n");

    while(getchar() != '\n');
    printf("\nPress Enter to return to the menu...");
    getchar();
}

// Estimates waiting time as queue length x consultation time of the specialty
float calculateWaitingTime(int specialityId, int queueCount[], int specialityTime[])
{
    return queueCount[specialityId - 1] * specialityTime[specialityId - 1];
}

// Calculates the emergency surcharge based on the urgency level
float calculateEmergencySurcharge(int urgencyLevel, float baseFee)
{
    if(urgencyLevel == URGENCY_NORMAL){
        return 0;
    }
    else if (urgencyLevel == URGENCY_URGENT){
        return baseFee * (SURCHARGE_URGENT / 100.0);
    }
    else{
        return baseFee * (SURCHARGE_CRITICAL / 100.0);
    }
}

// Calculates the total ward cost as days admitted x daily bed rate
float calculateTotalWardStayCost(int daysAdmitted, int wardId, float wardRate[])
{
    if (daysAdmitted == 0) {
        return 0;
    }
    else {
        return daysAdmitted * wardRate[wardId - 1];
    }
}

// Adds consultation fee, surcharge and ward cost to get the gross total
float calculateGrossTotalBill(float baseConsultanFee, float emegencySurcharge, float totalWardCost)
{
    return baseConsultanFee + emegencySurcharge + totalWardCost;
}

// Gives a 15% subsidy for patients younger than 5 or older than 65
float calculateSubsidyDiscount(int age, float grossTotal)
{
    if(age < AGE_YOUNG || age > AGE_ADULT){
        return grossTotal * (SUBSIDY_PERCENT / 100.0);
    }
    return 0;
}

// Subtracts the discount from the gross total
float calculateFinalAmountPayable(float grossTotal, float discount)
{
    return grossTotal - discount;
}

// Prints the formatted admission bill for a patient
void printBill(int patientId, char patientName[], int age, int specialtyId, int wardId, int isAdmitted,
               int urgencyLevel, int daysAdmitted, float baseFee, float surcharge, float wardCost,
               float grossTotal, float discount, float finalAmount, float waitingTime, int bedNum)
{
    printf("\n============================================================\n");
    printf("              SMART HOSPITAL ADMISSION & BILL\n");
    printf("============================================================\n");
    printf("%-28s: PAT-%d\n", "Patient ID", patientId);
    printf("%-28s: %s\n", "Patient Name", patientName);

    if(age < AGE_YOUNG || age > AGE_ADULT){
        printf("%-28s: %d Years (15%% Subsidy Eligible)\n", "Age", age);
    }
    else {
        printf("%-28s: %d Years\n", "Age", age);
    }

    printf("%-28s: %s\n", "Specialty", specialtyNames[specialtyId-1]);

    if(isAdmitted == 1){
        printf("%-28s: %s (Bed #%02d)\n", "Assigned Ward", wardName[wardId-1], bedNum);
    }
    else {
        printf("%-28s: Not Admitted (OPD)\n", "Assigned Ward");
    }

    if(urgencyLevel == 1){
        printf("%-28s: Level 1 (Normal)\n", "Urgency Level");
    }
    else if(urgencyLevel == 2){
        printf("%-28s: Level 2 (Urgent)\n", "Urgency Level");
    }
    else{
        printf("%-28s: Level 3 (Critical)\n", "Urgency Level");
    }

    printf("------------------------------------------------------------\n");
    printf("%-28s: LKR %.2f\n", "Base Consultation Fee", baseFee);

    if(urgencyLevel == 1){
        printf("%-28s: LKR %.2f\n", "Emergency Surcharge", surcharge);
    }
    else if(urgencyLevel == 2){
        printf("%-28s: LKR %.2f (20%%)\n", "Emergency Surcharge", surcharge);
    }
    else{
        printf("%-28s: LKR %.2f (50%%)\n", "Emergency Surcharge", surcharge);
    }

    if(isAdmitted == 1){
        char wardLabel[30];
        sprintf(wardLabel, "Ward Stay Cost (%d Days)", daysAdmitted);
        printf("%-28s: LKR %.2f\n", wardLabel, wardCost);
    }

    printf("------------------------------------------------------------\n");
    printf("%-28s: LKR %.2f\n", "Gross Total Bill", grossTotal);

    if(discount > 0){
        printf("%-28s: LKR -%.2f (15%%)\n", "Age Subsidy Discount", discount);
    }
    else{
        printf("%-28s: LKR -%.2f\n", "Age Subsidy Discount", discount);
    }

    printf("------------------------------------------------------------\n");
    printf("%-28s: LKR %.2f\n", "Final Payable Amount", finalAmount);

    if(waitingTime == 0){
        printf("%-28s: 0.00 mins (Immediate Attention)\n", "Estimated Waiting Time");
    }
    else {
        printf("%-28s: %.2f mins\n", "Estimated Waiting Time", waitingTime);
    }

    printf("============================================================\n");
}

// Writes the bed occupancy of every ward to beds_status.txt
void saveBedStatus(int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[])
{
    FILE *fp = fopen("beds_status.txt", "w");
    if(fp == NULL){
        printf("  >> Error: Could not save bed status.\n");
        return;
    }

    int w, b;
    for(w = 0; w < NUM_WARD; w++){
        for(b = 0; b < wardCapacity[w]; b++){
            fprintf(fp, "%d ", bedOccupancy[w][b]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

// Reads saved bed occupancy from beds_status.txt when the program starts
void loadBedStatus(int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[])
{
    FILE *fp = fopen("beds_status.txt", "r");
    if(fp == NULL){
        return;
    }

    int w, b;
    for(w = 0; w < NUM_WARD; w++){
        for(b = 0; b < wardCapacity[w]; b++){
            fscanf(fp, "%d", &bedOccupancy[w][b]);
        }
    }

    fclose(fp);
}

// Appends one line with patient ID, name and final bill to patient_records.txt
void appendPatientRecord(int patientId, char name[], float finalAmount)
{
    FILE *fp = fopen("patient_records.txt", "a");
    if(fp == NULL){
        printf("  >> Error: Could not save patient record.\n");
        return;
    }

    fprintf(fp, "PAT-%-6d | %-20s | LKR %10.2f\n", patientId, name, finalAmount);

    fclose(fp);
}

	// Reads patient details, allocates a bed if admitted, calculates the bill and prints it
void registerPatients(char patientName[][NAME_LENGTH],
                      int patientAge[],
                      int patientEmergencyLevel[],
                      int specialtyId[],
                      int isAdmitted[],
                      int wardId[],
                      int daysAdmitted[],
                      int specialtyQueueCount[],
                      float patientFinalBill[],
                      float patientDiscount[],
                      int *count)
{
    int i = *count;
    if(i >= MAX_PATIENTS){
        printf("\n  >> Patient limit reached. Cannot register more patients.\n");
        return;
    }

    printf("\n============================================================\n");
    printf("                   PATIENT REGISTRATION\n");
    printf("============================================================\n");
    printf("   1. Mr.\n");
    printf("   2. Mrs.\n");
    printf("   3. Miss\n");
    int titleChoice = getValidInt(1, 3, "Title");

    char titleText[10];
    if(titleChoice == 1){
        strcpy(titleText, "Mr.");
    }
    else if(titleChoice == 2){
        strcpy(titleText, "Mrs.");
    }
    else{
        strcpy(titleText, "Miss");
    }

    char fullName[NAME_LENGTH];
    printf("%-20s: ", "Patient Name");
    scanf(" %40[^\n]", fullName);

    sprintf(patientName[i], "%s %s", titleText, fullName);

    patientAge[i] = getValidInt(0, 120, "Age of Patient");

    printf("\n------------------------------------------------------------\n");
    printf("                  SELECT EMERGENCY LEVEL\n");
    printf("------------------------------------------------------------\n");
    printf("   1. Normal\n");
    printf("   2. Urgent\n");
    printf("   3. Critical\n");
    patientEmergencyLevel[i] = getValidInt(1, 3, "Emergency Level");

    printf("\n------------------------------------------------------------\n");
    printf("                    SELECT SPECIALTY\n");
    printf("------------------------------------------------------------\n");
    printf("   1. General Practice (OPD)\n");
    printf("   2. Paediatrics\n");
    printf("   3. Cardiology\n");
    printf("   4. Neurology\n");
    specialtyId[i] = getValidInt(1, 4, "Specialty ID");

    printf("\n------------------------------------------------------------\n");
    printf("                   PATIENT ADMISSION\n");
    printf("------------------------------------------------------------\n");
    printf("   1. Admitted\n");
    printf("   0. Not Admitted\n");
    isAdmitted[i] = getValidInt(0, 1, "Admitted (0/1)");

    int bedNum = 0;
    if(isAdmitted[i] == 1){
        printf("\n------------------------------------------------------------\n");
        printf("                     SELECT WARD ID\n");
        printf("------------------------------------------------------------\n");
        printf("   1. General Ward\n");
        printf("   2. Paediatric Ward\n");
        printf("   3. Surgical Ward\n");
        printf("   4. ICU (Intensive Care Unit)\n");
        wardId[i] = getValidInt(1, 4, "Ward ID");

        daysAdmitted[i] = getValidInt(1, 9999, "Days Admitted");

        bedNum = allocateBed(wardId[i], bedOccupancy, wardTotalBedCapacity);
        if(bedNum == -1){
            printf("\n  >> Sorry, %s is Full. No beds available.\n", wardName[wardId[i]-1]);
                    if(bedNum == -1){
            printf("\n  >> Sorry, %s is Full. No beds available.\n", wardName[wardId[i]-1]);
            isAdmitted[i] = 0;
            wardId[i] = 0;
            daysAdmitted[i] = 0;
            bedNum = 0;
        }
        }
    } else {
        wardId[i] = 0;
        daysAdmitted[i] = 0;
    }

    printf("\n============================================================\n");
    printf("  Patient Registered Successfully! Patient ID: PAT-%d\n", 1000 + (i + 1));
    printf("============================================================\n");
    (*count)++;


    float waitingTime = calculateWaitingTime(specialtyId[i], specialtyQueueCount, specialtyConslationTime);
    specialtyQueueCount[specialtyId[i]-1]++;
    float baseFee = specialtyConslationFee[specialtyId[i]-1];
    float surcharge = calculateEmergencySurcharge(patientEmergencyLevel[i], baseFee);
    float wardCost = calculateTotalWardStayCost(daysAdmitted[i], wardId[i], wardDailyBedRate);
    float grossTotal = calculateGrossTotalBill(baseFee, surcharge, wardCost);
    float discount = calculateSubsidyDiscount(patientAge[i], grossTotal);
    float finalAmount = calculateFinalAmountPayable(grossTotal, discount);


    patientFinalBill[i] = finalAmount;
    patientDiscount[i] = discount;
    appendPatientRecord(1000 + (i + 1), patientName[i], finalAmount);


    printBill(1000 + (i + 1), patientName[i], patientAge[i], specialtyId[i], wardId[i], isAdmitted[i],
              patientEmergencyLevel[i], daysAdmitted[i], baseFee, surcharge, wardCost,
              grossTotal, discount, finalAmount, waitingTime, bedNum);

    while(getchar() != '\n');
    printf("\nPress Enter to return to the menu...");
    getchar();
}

// Sorts patients by emergency level (highest first) using selection sort and displays them
void sortAndDisplayByPriority(char patientName[][NAME_LENGTH], int patientEmergencyLevel[], int count)
{
    int sortedIndex[MAX_PATIENTS];
    int i, j, k, maxId;

    for(i = 0; i < count; i++){
        sortedIndex[i] = i;
    }

    for (j = 0; j < count - 1; j++){
        maxId = j;
        for(i = j + 1; i < count; i++){
            if(patientEmergencyLevel[sortedIndex[i]] > patientEmergencyLevel[sortedIndex[maxId]]){
                maxId = i;
            }
        }
        k = sortedIndex[j];
        sortedIndex[j] = sortedIndex[maxId];
        sortedIndex[maxId] = k;
    }

    printf("\n============================================================\n");
    printf("                 PATIENTS BY PRIORITY\n");
    printf("============================================================\n");
    printf("%-10s| %-20s| %s\n", "Patient ID", "Name", "Urgency Level");
    printf("------------------------------------------------------------\n");
    for(j = 0; j < count; j++){
        int index = sortedIndex[j];
        char patId[10];
        sprintf(patId, "PAT-%d", 1000 + index + 1);
        printf("%-10s| %-20s| Level %d\n", patId, patientName[index], patientEmergencyLevel[index]);
    }
    printf("============================================================\n");

    while(getchar() != '\n');
    printf("\nPress Enter to return to the menu...");
    getchar();
}

// Shows patient counts, revenue, discounts, bed occupancy % and the highest paying patient
void generateSummaryReport(int patientEmergencyLevel[], float patientFinalBill[], float patientDiscount[],
                            char patientName[][NAME_LENGTH], int count,
                            int bedOccupancy[][MAX_BED_PER_WARD], int wardCapacity[])
{
    int k, w, b;

    printf("\n============================================================\n");
    printf("               HOSPITAL PERFORMANCE REPORT\n");
    printf("============================================================\n");


    int normalCount = 0, urgentCount = 0, criticalCount = 0;
    for(k = 0; k < count; k++){
        if(patientEmergencyLevel[k] == 1) normalCount++;
        else if(patientEmergencyLevel[k] == 2) urgentCount++;
        else criticalCount++;
    }
    printf("\n%-28s: %d\n", "Total Patients Registered", count);
    printf("%-28s: %d\n", "  Normal", normalCount);
    printf("%-28s: %d\n", "  Urgent", urgentCount);
    printf("%-28s: %d\n", "  Critical", criticalCount);


    float totalRevenue = 0, totalDiscount = 0;
    for(k = 0; k < count; k++){
        totalRevenue += patientFinalBill[k];
        totalDiscount += patientDiscount[k];
    }
    printf("\n%-28s: LKR %.2f\n", "Total Revenue Earned", totalRevenue);
    printf("%-28s: LKR %.2f\n", "Total Discounts Granted", totalDiscount);

    printf("\n------------------------------------------------------------\n");
    printf("Bed Occupancy Percentage:\n");
    printf("------------------------------------------------------------\n");
    for(w = 0; w < NUM_WARD; w++){
        int occupied = 0;
        for(b = 0; b < wardCapacity[w]; b++){
            if(bedOccupancy[w][b] == 1) occupied++;
        }
        float percent = (occupied * 100.0) / wardCapacity[w];
        printf("%-28s: %.2f%%\n", wardName[w], percent);
    }

    if(count > 0){
        int highestIdx = 0;
        for(k = 1; k < count; k++){
            if(patientFinalBill[k] > patientFinalBill[highestIdx]){
                highestIdx = k;
            }
        }
        printf("\n%-28s: %s - LKR %.2f\n", "Highest Paying Patient", patientName[highestIdx], patientFinalBill[highestIdx]);
    }

    printf("\n============================================================\n");

    while(getchar() != '\n');
    printf("\nPress Enter to return to the menu...");
    getchar();
}

int main()
{
    int choice;

    char patientName[MAX_PATIENTS][NAME_LENGTH];
    int patientAge[MAX_PATIENTS];
    int patientEmergencyLevel[MAX_PATIENTS];
    int specialtyId[MAX_PATIENTS];
    int isAdmitted[MAX_PATIENTS];
    int wardId[MAX_PATIENTS];
    int daysAdmitted[MAX_PATIENTS];

    loadBedStatus(bedOccupancy, wardTotalBedCapacity);

    do {

        printf("\n============================================================\n");
        printf("          SMART HOSPITAL & RESOURCE ALLOCATION SYSTEM\n");
        printf("============================================================\n");
        printf("   1. Register Patient\n");
        printf("   2. Check Bed Status\n");
        printf("   3. View Patients by Priority\n");
        printf("   4. Generate Performance Report\n");
        printf("   5. Exit\n");
        printf("============================================================\n");
        choice = getValidInt(1, 5, "Enter Your Choice");
       

        switch(choice){
            case 1:
                registerPatients(patientName, patientAge, patientEmergencyLevel, specialtyId, isAdmitted, wardId,
                                 daysAdmitted, specialtyQueueCount, patientFinalBill, patientDiscount, &patientCount);
                break;
            case 2:
                checkBedStatus(bedOccupancy, wardTotalBedCapacity);
                break;
            case 3:
                sortAndDisplayByPriority(patientName, patientEmergencyLevel, patientCount);
                break;
            case 4:
                generateSummaryReport(patientEmergencyLevel, patientFinalBill, patientDiscount,
                                     patientName, patientCount, bedOccupancy, wardTotalBedCapacity);
                break;
            case 5:
                saveBedStatus(bedOccupancy, wardTotalBedCapacity);
                printf("\nThank you for using Smart Hospital System....\n");
                break;
            default:
                printf("\n  >> Invalid choice. Please try again.\n");
        }
    } while(choice != 5);

    return 0;
}
