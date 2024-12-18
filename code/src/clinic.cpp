/**
 * @file clinic.cpp
 * @author Berberat Alex
 * @author Surbeck Léon
 * @brief Implementation of clinic fonctions.
 */

#include "clinic.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>
#include <variant>


IWindowInterface *Clinic::interface = nullptr;

Clinic::Clinic(int uniqueId, int fund, std::vector<ItemType> resourcesNeeded)
    : Seller(fund, uniqueId), nbTreated(0), resourcesNeeded(resourcesNeeded) {
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Factory created");

    for (const auto &item : resourcesNeeded) {
        stocks[item] = 0;
    }
}

bool Clinic::verifyResources() {
    for (auto item : resourcesNeeded) {
        if (stocks[item] == 0) {
            return false;
        }
    }
    return true;
}

int Clinic::request(ItemType what, int qty) {
    // DONE
    clinic_mutex.lock();
    int bill = getCostPerUnit(ItemType::PatientHealed) * qty;

    if (stocks[what] >= qty) {
        stocks[what] -= qty;
        money += bill;
        clinic_mutex.unlock();
        return bill;
    }
    clinic_mutex.unlock();
    return 0;
}

void Clinic::treatPatient() {
    // DONE
    int bill = getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));

    clinic_mutex.lock();
    if (money >= bill) {
        money -= bill;
        //Temps simulant un traitement
        interface->simulateWork();

        // DONE
        for (auto item : resourcesNeeded) {
            --stocks[item];
        }
        ++stocks[ItemType::PatientHealed];

        ++nbTreated;
        interface->consoleAppendText(uniqueId, "Clinic have healed a new patient");
    }

    clinic_mutex.unlock();
}

void Clinic::orderResources() {
    // DONE
    auto randHosp = chooseRandomSeller(hospitals);
    int qty = 1;
    auto sick = ItemType::PatientSick;
    int bill = getCostPerUnit(sick) * qty;
    if (money >= bill) {
        if (randHosp->request(sick, qty) != 0) {
            money -= bill;
            stocks[sick] += qty;
        }
    }


    auto randSupp = chooseRandomSeller(suppliers);
    for (auto i : resourcesNeeded) {
        if (i != ItemType::PatientSick && money - getCostPerUnit(i) * qty >= 0) {
            if (randSupp->request(i, qty)) {
                stocks[i] += qty;
                money -= getCostPerUnit(i) * qty;
            }
        }
    }
}

void Clinic::run() {
    if (hospitals.empty() || suppliers.empty()) {
        std::cerr << "You have to give to hospitals and suppliers to run a clinic" << std::endl;
        return;
    }
    interface->consoleAppendText(uniqueId, "[START] Factory routine");

    while (!PcoThread::thisThread()->stopRequested()) {
        clinic_mutex.lock();
        if (verifyResources()) {
            treatPatient();
        } else {
            orderResources();
        }
        clinic_mutex.unlock();

        interface->simulateWork();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Factory routine");
}


void Clinic::setHospitalsAndSuppliers(std::vector<Seller *> hospitals, std::vector<Seller *> suppliers) {
    this->hospitals = hospitals;
    this->suppliers = suppliers;

    for (Seller *hospital : hospitals) {
        interface->setLink(uniqueId, hospital->getUniqueId());
    }
    for (Seller *supplier : suppliers) {
        interface->setLink(uniqueId, supplier->getUniqueId());
    }
}

int Clinic::getTreatmentCost() {
    return 0;
}

int Clinic::getWaitingPatients() {
    return stocks[ItemType::PatientSick];
}

int Clinic::getNumberPatients() {
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed];
}

int Clinic::send(ItemType it, int qty, int bill) {
    return 0;
}

int Clinic::getAmountPaidToWorkers() {
    return nbTreated * getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));
}

void Clinic::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

std::map<ItemType, int> Clinic::getItemsForSale() {
    return stocks;
}


Pulmonology::Pulmonology(int uniqueId, int fund) : Clinic::Clinic(uniqueId, fund, { ItemType::PatientSick, ItemType::Pill, ItemType::Thermometer }) {
}

Cardiology::Cardiology(int uniqueId, int fund) : Clinic::Clinic(uniqueId, fund, { ItemType::PatientSick, ItemType::Syringe, ItemType::Stethoscope }) {
}

Neurology::Neurology(int uniqueId, int fund) : Clinic::Clinic(uniqueId, fund, { ItemType::PatientSick, ItemType::Pill, ItemType::Scalpel }) {
}
