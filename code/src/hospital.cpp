/**
 * @file hospital.cpp
 * @author Berberat Alex
 * @author Surbeck Léon
 * @brief Implementation of hospital fonctions.
 */

#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <pcosynchro/pcothread.h>
#include <sys/stat.h>

#include "utils.h"

IWindowInterface *Hospital::interface = nullptr;

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : Seller(fund, uniqueId), maxBeds(maxBeds), currentBeds(0), nbHospitalised(0), nbFree(0) {
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Hospital Created with " + QString::number(maxBeds) + " beds");

    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for (const auto &item : initialStocks) {
        stocks[item] = 0;
    }
}

int Hospital::request(ItemType what, int qty) {
    // DONE
    hospital_mutex.lock();
    int bill = getCostPerUnit(ItemType::PatientSick) * qty;

    if (stocks[what] >= qty) {
        stocks[what] -= qty;
        money += bill;
        hospital_mutex.unlock();
        return bill;
    }
    hospital_mutex.unlock();
    return 0;
}

void Hospital::freeHealedPatient() {
    // DONE
    hospital_mutex.lock();
    if (stocks[ItemType::PatientHealed] > 0) {
        for (auto &i : healedPatientsDaysLeft) {
            --i;
            if (i == 0) {
                stocks[ItemType::PatientHealed] -= 1;
                interface->consoleAppendText(uniqueId, "Hospital has freed a new patient");
                ++nbFree;
            }
        }
        healedPatientsDaysLeft.erase(std::remove(healedPatientsDaysLeft.begin(), healedPatientsDaysLeft.end(), 0), healedPatientsDaysLeft.end());
        healedPatientsDaysLeft.shrink_to_fit();
    }
    hospital_mutex.unlock();
}

void Hospital::transferPatientsFromClinic() {
    // DONE

    hospital_mutex.lock();
    if (stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed] < MAX_BEDS_PER_HOSTPITAL) {
        auto randClinic = this->chooseRandomSeller(clinics);
        int bill = randClinic->request(ItemType::PatientHealed, 1);
        if (bill > 0 && bill + getEmployeeSalary(EmployeeType::Nurse) <= money) {
            stocks[ItemType::PatientHealed] += 1;
            ++nbHospitalised;
            money -= bill;
            money -= getEmployeeSalary(EmployeeType::Nurse);
            healedPatientsDaysLeft.push_back(5);
        }
    }

    hospital_mutex.unlock();
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
 * \return The bill amount if the item was successfully sent, 0 otherwise.
 */
int Hospital::send(ItemType it, int qty, int bill) {
    hospital_mutex.lock();
    if (money >= bill + getEmployeeSalary(EmployeeType::Nurse) && stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed] + qty <= maxBeds) {
        nbHospitalised += qty;
        money -= bill;
        money -= getEmployeeSalary(EmployeeType::Nurse);
        stocks[it] += qty;
        hospital_mutex.unlock();
        return bill;
    }

    hospital_mutex.unlock();
    return 0;
}

void Hospital::run() {
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

int Hospital::getNumberPatients() {
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed] + nbFree;
}

std::map<ItemType, int> Hospital::getItemsForSale() {
    return stocks;
}

void Hospital::setClinics(std::vector<Seller *> clinics) {
    this->clinics = clinics;

    for (Seller *clinic : clinics) {
        interface->setLink(uniqueId, clinic->getUniqueId());
    }
}

void Hospital::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}
