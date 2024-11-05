/**
 * @file tests_main.cpp
 * @author Berberat Alex
 * @author Surbeck Léon
 * @brief Unit test to test if the different aspect of the project
 *      work as intended.
 */

#include <gtest/gtest.h>
#include "supplier.h"
#include "ambulance.h"
#include "iwindowinterface.h"
#include "fakeinterface.h"
#include <pcosynchro/pcothread.h>
#include <iostream>
#include <vector>
#include <random>
#include "utils.h"
#include "hospital.h"
#include "clinic.h"
#include <atomic>

 // Helper functions for testing
void sendStuff(Seller &seller, ItemType itemType, std::atomic<int> &totalPaid) {
    int tot = 0;
    for (int i = 0; i < 1000; ++i) { // Reduced loop count to avoid excessive processing
        int qty = 1;
        int bill = getCostPerUnit(itemType) * qty;
        if (seller.send(itemType, qty, bill) > 0) {
            tot += bill;
        }
    }
    totalPaid += tot;
}

void requestStuff(Seller &seller, ItemType itemType, std::atomic<int> &totalGained) {
    int tot = 0;
    for (int i = 0; i < 1000; ++i) { // Reduced loop count to avoid excessive processing
        int qty = 1;
        tot += seller.request(itemType, qty);
    }
    totalGained += tot;
}

void sendPatient(Ambulance &ambulance, std::atomic<int> &totalGained) {
    int tot = 0;
    for (int i = 0; i < 1000; ++i) { // Reduced loop count to avoid excessive processing
        int bill = ambulance.sendPatientTest();
        tot += bill;
    }
    totalGained += tot;
}

// Test for Hospital class
TEST(SellerTest, TestHospitals) {
    const int uniqueId = 0;
    const int initialFund = 20000;
    const unsigned int nbThreads = 4;
    int endFund = 0;
    std::atomic<int> totalPaid = 0;
    std::atomic<int> totalGained = 0;

    IWindowInterface *windowInterface = new FakeInterface();
    Hospital::setInterface(windowInterface);

    Hospital hospital(uniqueId, initialFund, 50); // Hospital with 50 beds

    std::vector<std::unique_ptr<PcoThread>> threads;

    for (unsigned int i = 0; i < nbThreads / 2; ++i) {
        threads.emplace_back(std::make_unique<PcoThread>(requestStuff, std::ref(hospital), ItemType::PatientSick, std::ref(totalGained)));
        threads.emplace_back(std::make_unique<PcoThread>(sendStuff, std::ref(hospital), ItemType::PatientSick, std::ref(totalPaid)));
    }

    for (auto &thread : threads) {
        thread->join();
    }

    endFund += hospital.getFund();
    endFund += hospital.getAmountPaidToWorkers();
    endFund += totalPaid;
    endFund -= totalGained;

    EXPECT_EQ(endFund, initialFund);
    EXPECT_GE(hospital.getNumberPatients(), 0);
    EXPECT_LE(hospital.getNumberPatients(), 50);
}

// Test for Ambulance class
TEST(SellerTest, TestAmbulance) {
    const int uniqueId = 1;
    const int initialFund = 20000;
    const unsigned int nbThreads = 4;
    int endFund = 0;
    std::atomic<int> totalGained = 0;

    IWindowInterface *windowInterface = new FakeInterface();
    Ambulance::setInterface(windowInterface);

    Ambulance ambulance(uniqueId, initialFund, { ItemType::PatientSick }, { {ItemType::PatientSick, 1000} });

    std::vector<std::unique_ptr<PcoThread>> threads;

    for (unsigned int i = 0; i < nbThreads / 2; ++i) {
        threads.emplace_back(std::make_unique<PcoThread>(sendPatient, std::ref(ambulance), std::ref(totalGained)));
    }

    for (auto &thread : threads) {
        thread->join();
    }

    endFund += ambulance.getFund();
    endFund += ambulance.getAmountPaidToWorkers();
    endFund -= totalGained;

    EXPECT_GE(endFund, initialFund);
    EXPECT_GE(ambulance.getNumberPatients(), 0);
}

void requestSupplies(Supplier &supplier, ItemType itemType, std::atomic<int> &totalGained) {
    int tot = 0;
    for (int i = 0; i < 1000; ++i) { // Reduced loop count to avoid excessive processing
        int qty = 1;
        tot += supplier.request(itemType, qty);
    }
    totalGained += tot;
}

// Test for Supplier class
TEST(SellerTest, TestSupplier) {
    const int uniqueId = 2;
    const int initialFund = 30000;
    std::atomic<int> totalGained = 0;

    IWindowInterface *windowInterface = new FakeInterface();
    Supplier::setInterface(windowInterface);

    Supplier supplier(uniqueId, initialFund, { ItemType::Syringe, ItemType::Pill });

    std::vector<std::unique_ptr<PcoThread>> threads;
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(std::make_unique<PcoThread>(requestSupplies, std::ref(supplier), ItemType::Syringe, std::ref(totalGained)));
    }

    for (auto &thread : threads) {
        thread->join();
    }

    EXPECT_GE(totalGained, 0);
}

// Test for Clinic class
TEST(SellerTest, TestClinic) {
    const int uniqueId = 3;
    const int initialFund = 15000;
    const unsigned int nbThreads = 4;
    std::atomic<int> totalPaid = 0;
    std::atomic<int> totalGained = 0;

    IWindowInterface *windowInterface = new FakeInterface();
    Clinic::setInterface(windowInterface);

    Clinic clinic(uniqueId, initialFund, { ItemType::PatientSick, ItemType::Pill, ItemType::Thermometer });

    std::vector<std::unique_ptr<PcoThread>> threads;
    for (int i = 0; i < nbThreads / 2; ++i) {
        threads.emplace_back(std::make_unique<PcoThread>(sendStuff, std::ref(clinic), ItemType::PatientSick, std::ref(totalPaid)));
        threads.emplace_back(std::make_unique<PcoThread>(requestStuff, std::ref(clinic), ItemType::PatientHealed, std::ref(totalGained)));
    }

    for (auto &thread : threads) {
        thread->join();
    }

    int endFund = clinic.getFund() + clinic.getAmountPaidToWorkers() + totalPaid - totalGained;

    EXPECT_EQ(endFund, initialFund);
    EXPECT_GE(clinic.getNumberPatients(), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
