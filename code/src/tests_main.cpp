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
void sendPatients(Hospital& hospital, ItemType itemType, std::atomic<int>& totalPaid) {
    int tot = 0;
    for (int i = 0; i < 20000; ++i) {
        int qty = 1;
        int bill = getCostPerUnit(itemType) * qty;
        if (hospital.send(itemType, qty, bill) > 0) {
            tot += bill;
        }
    }
    totalPaid += tot;
}

void requestPatients(Hospital& hospital, ItemType itemType, std::atomic<int>& totalGained) {
    int tot = 0;
    for (int i = 0; i < 20000; ++i) {
        int qty = 1;
        tot += hospital.request(itemType, qty);
    }
    totalGained += tot;
}

// Test for Hospital class
TEST(SellerTest, TestHospitals) {
    const int uniqueId = 0;
    const int initialFund = 20000;
    const unsigned int maxBeds = MAX_BEDS_PER_HOSTPITAL;
    const unsigned int nbThreads = 4;
    int endFund = 0;
    std::atomic<int> totalPaid = 0;
    std::atomic<int> totalGained = 0;

    IWindowInterface* windowInterface = new FakeInterface();
    Hospital::setInterface(windowInterface);

    Hospital hospital(uniqueId, initialFund, maxBeds);

    std::vector<std::unique_ptr<PcoThread>> threads;

    for (unsigned int i = 0; i < nbThreads / 2; ++i) {
        threads.emplace_back(std::make_unique<PcoThread>(requestPatients, std::ref(hospital), ItemType::PatientSick, std::ref(totalGained)));
        threads.emplace_back(std::make_unique<PcoThread>(sendPatients, std::ref(hospital), ItemType::PatientSick, std::ref(totalPaid)));
    }

    for (auto& thread : threads) {
        thread->join();
    }

    endFund += hospital.getFund();
    endFund += hospital.getAmountPaidToWorkers();
    endFund += totalPaid;
    endFund -= totalGained;

    EXPECT_EQ(endFund, initialFund);
    EXPECT_GE(hospital.getNumberPatients(), 0);
    EXPECT_LE(hospital.getNumberPatients(), maxBeds);
}









void requestSupplies(Supplier& supplier, ItemType itemType, std::atomic<int>& totalGained){
    int tot = 0;
    for (int i = 0; i < 20000; ++i) {
        int qty = 1;
        tot += supplier.request(itemType, qty);
    }

    totalGained += tot;
}

// Test for Supplier class
TEST(SellerTest, TestSupplier){



}





void sendAmbulancePatients(Ambulance& ambulance, std::atomic<int>& totalGained) {
    for (int i = 0; i < 20000; ++i) {
        ambulance.sendPatient();
        totalGained += getCostPerUnit(ItemType::PatientSick);
    }
}


// Test for Ambulance class
TEST(SellerTest, TestAmbulance){



}









// Test for Clinic class
TEST(SellerTest, TestClinic){


}













int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
