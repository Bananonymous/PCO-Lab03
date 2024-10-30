#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

#include "utils.h"

IWindowInterface* Hospital::interface = nullptr;

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : Seller(fund, uniqueId), maxBeds(maxBeds), currentBeds(0), nbHospitalised(0), nbFree(0)
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Hospital Created with " + QString::number(maxBeds) + " beds");
    
    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for(const auto& item : initialStocks) {
        stocks[item] = 0;
    }
}

int Hospital::request(ItemType what, int qty){
    // TODO
    int bill = getEmployeeSalary(getEmployeeThatProduces(what));
    if(stocks[what] >= qty) {
        stocks[what] -= qty;
        money += bill;
        return 1;
    }
    return 0;
}

void Hospital::freeHealedPatient() {
    // TODO 
}

void Hospital::transferPatientsFromClinic() {
    // TODO
}

/**
 * \brief Sends a specified quantity of an item to the hospital.
 *
 * This function attempts to send a specified quantity of an item to the hospital.
 * It checks if the hospital has enough funds to cover the bill and if there are enough beds available.
 * If both conditions are met, the function updates the number of hospitalized patients, deducts the bill from the hospital's funds,
 * and increases the stock of the specified item.
 *
 * @param it The type of item to be sent.
 * @param qty The quantity of the item to be sent.
 * @param bill The cost associated with sending the item.
 * \return 1 if the item was successfully sent, 0 otherwise.
 */
int Hospital::send(ItemType it, int qty, int bill) {
    if(money-bill >= 0 && this->getNumberPatients() + qty <= this->maxBeds) {
        nbHospitalised += qty;
        money -= bill;
        stocks[it] += qty;
        return 1;
    }
    return 0;
}

void Hospital::run()
{
    if (clinics.empty()) {
        std::cerr << "You have to give clinics to a hospital before launching is routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Hospital routine");
    
    //Update to this while() so it stops when requested to.
    while (!PcoThread::thisThread()->stopRequested()) {
        transferPatientsFromClinic();

        freeHealedPatient();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
        interface->simulateWork(); // Temps d'attente
    }

    interface->consoleAppendText(uniqueId, "[STOP] Hospital routine");
}

int Hospital::getAmountPaidToWorkers() {
    return nbHospitalised * getEmployeeSalary(EmployeeType::Nurse);
}

int Hospital::getNumberPatients(){
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed] + nbFree;
}

std::map<ItemType, int> Hospital::getItemsForSale()
{
    return stocks;
}

void Hospital::setClinics(std::vector<Seller*> clinics){
    this->clinics = clinics;

    for (Seller* clinic : clinics) {
        interface->setLink(uniqueId, clinic->getUniqueId());
    }
}

void Hospital::setInterface(IWindowInterface* windowInterface){
    interface = windowInterface;
}
