/**
 * @file ambulance.cpp
 * @author Berberat Alex
 * @author Surbeck Léon
 * @brief Implement all the fonctions to be able to handle ambulance.
 */

#include "ambulance.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

IWindowInterface *Ambulance::interface = nullptr;

Ambulance::Ambulance(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied, std::map<ItemType, int> initialStocks)
    : Seller(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbTransfer(0) {
    interface->consoleAppendText(uniqueId, QString("Ambulance Created"));

    for (const auto &item : resourcesSupplied) {
        if (initialStocks.find(item) != initialStocks.end()) {
            stocks[item] = initialStocks[item];
        } else {
            stocks[item] = 0;
        }
    }

    interface->updateFund(uniqueId, fund);
}

int Ambulance::sendPatientTest() {

    if(stocks[ItemType::PatientSick] <= 0) {
        return 0;
    }

    int bill = getCostPerUnit(ItemType::PatientSick);
    int patientSent = bill; //Simulate sending a patient

    ambulance_mutex.lock();
    if(patientSent > 0) {
        money += bill;
        --stocks[ItemType::PatientSick];
        ++nbTransfer;
        money -= getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientSick));
        ambulance_mutex.unlock();
        return bill;
    }
    ambulance_mutex.unlock();
}

/**
 * \brief Sends a patient to a randomly chosen hospital.
 *
 * This function selects a random hospital from the list of hospitals and attempts to send a sick patient to it.
 * If the hospital accepts the patient, the ambulance's money is increased by the salary of the employee who produces
 * the sick patient, the stock of sick patients is decreased by one, and the number of transfers is incremented.
 */
void Ambulance::sendPatient() {
    // DONE
    ambulance_mutex.lock();

    auto randHosp = this->chooseRandomSeller(hospitals);
    int qtyPatients = 1;
    int bill = getCostPerUnit(ItemType::PatientSick) * qtyPatients;

    if (stocks[ItemType::PatientSick] >= qtyPatients) {
        if (randHosp->send(ItemType::PatientSick, qtyPatients, bill)) {
            money += bill;

            if (money >= getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientSick)) * qtyPatients)
                money -= getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientSick)) * qtyPatients;

            this->stocks[ItemType::PatientSick] -= qtyPatients;

            nbTransfer += qtyPatients;
        }
    }
    ambulance_mutex.unlock();
}

void Ambulance::run() {
    interface->consoleAppendText(uniqueId, "[START] Ambulance routine");

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
    for (const auto &item : resourcesSupplied) {
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

    for (Seller *hospital : hospitals) {
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
