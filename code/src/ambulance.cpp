#include "ambulance.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

IWindowInterface *Ambulance::interface = nullptr;

Ambulance::Ambulance(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied, std::map<ItemType, int> initialStocks)
    : Seller(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbTransfer(0) {
    interface->consoleAppendText(uniqueId, QString("Ambulance Created"));

    for (const auto &item: resourcesSupplied) {
        if (initialStocks.find(item) != initialStocks.end()) {
            stocks[item] = initialStocks[item];
        } else {
            stocks[item] = 0;
        }
    }

    interface->updateFund(uniqueId, fund);
}

/**
 * \brief Sends a patient to a randomly chosen hospital.
 *
 * This function selects a random hospital from the list of hospitals and attempts to send a sick patient to it.
 * If the hospital accepts the patient, the ambulance's money is increased by the salary of the employee who produces
 * the sick patient, the stock of sick patients is decreased by one, and the number of transfers is incremented.
 */
void Ambulance::sendPatient() {
    auto randHosp = this->chooseRandomSeller(hospitals);
    if (randHosp->send(ItemType::PatientSick, 10, 4)) {
        money += getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientSick));

        this->stocks[ItemType::PatientSick] -= 1;

        ++nbTransfer;
    }
}

void Ambulance::run() {
    interface->consoleAppendText(uniqueId, "[START] Ambulance routine");

    //Update to this while() so it stops when requested to.
    while (!PcoThread::thisThread()->stopRequested()) {
        sendPatient();
        interface->simulateWork();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }

    interface->consoleAppendText(uniqueId, "[STOP] Ambulance routine");
}

std::map<ItemType, int> Ambulance::getItemsForSale() {
    return stocks;
}

int Ambulance::getMaterialCost() {
    int totalCost = 0;
    for (const auto &item: resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

int Ambulance::getAmountPaidToWorkers() {
    return nbTransfer * getEmployeeSalary(EmployeeType::Supplier);
}

int Ambulance::getNumberPatients() {
    return stocks[ItemType::PatientSick];
}

void Ambulance::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}


void Ambulance::setHospitals(std::vector<Seller *> hospitals) {
    this->hospitals = hospitals;

    for (Seller *hospital: hospitals) {
        interface->setLink(uniqueId, hospital->getUniqueId());
    }
}

int Ambulance::send(ItemType it, int qty, int bill) {
    return 0;
}


int Ambulance::request(ItemType what, int qty) {
    return 0;
}

std::vector<ItemType> Ambulance::getResourcesSupplied() const {
    return resourcesSupplied;
}
