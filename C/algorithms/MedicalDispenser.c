/*
 * Algorithm:
 * 1. Read N medicine records dynamically
 * 2. For each medicine calculate days of stock remaining
 * 3. Flag LOW STOCK if days < STOCK_THRESH
 * 4. Flag HIGH DOSAGE if dosage > DOSAGE_THRESH
 * 5. Track medicine with least days remaining
 * 6. Print summary and critical medicine at end
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STOCK_THRESH  3
#define DOSAGE_THRESH 500

typedef struct {
    char medicine[100];
    int  dosage;
    int  freq;
    int  stock;
} Dispenser;

int main() {
    int N = 0;
    printf("Enter number of medicines: ");
    scanf("%d", &N);

    Dispenser *Entries = (Dispenser*)malloc(N * sizeof(Dispenser));
    if(Entries == NULL) {
        printf("[FATAL] Memory allocation failed\n");
        return 1;
    }

    printf("Enter Medicine, Dosage, Frequency, Stock\n");
    for(int i = 0; i < N; i++) {
        scanf("%s %d %d %d", Entries[i].medicine,
              &Entries[i].dosage, &Entries[i].freq, &Entries[i].stock);
    }

    int  leastDay = Entries[0].stock / Entries[0].freq;
    char leastMedicine[100];
    strcpy(leastMedicine, Entries[0].medicine);

    for(int i = 0; i < N; i++) {
        int stockDays = Entries[i].stock / Entries[i].freq;
        int StockFlag = (stockDays < STOCK_THRESH) ? 1 : 0;
        int DoseFlag  = (Entries[i].dosage > DOSAGE_THRESH) ? 1 : 0;

        printf("%s: %d days left", Entries[i].medicine, stockDays);

        if(StockFlag && DoseFlag)
            printf(" — LOW STOCK, HIGH DOSAGE — VERIFY\n");
        else if(StockFlag)
            printf(" — LOW STOCK\n");
        else if(DoseFlag)
            printf(" — HIGH DOSAGE — VERIFY\n");
        else
            printf("\n");

        if(stockDays < leastDay) {
            leastDay = stockDays;
            strcpy(leastMedicine, Entries[i].medicine);
        }
    }

    printf("\nCritical: %s runs out first (%d days left)\n",
           leastMedicine, leastDay);

    free(Entries);
    return 0;
}
