/**
 * @file supplier.cpp
 * @author Berberat Alex
 * @author Surbeck Léon
 * @brief Implementation of supplier fonctions.
 */

#include "supplier.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

IWindowInterface *Supplier::interface = nullptr;

Supplier::Supplier(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied)
    : Seller(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbSupplied(0) {
    for (const auto &item : resourcesSupplied) {
        stocks[item] = 0;
    }

    interface->consoleAppendText(uniqueId, QString("Supplier Created"));
    interface->updateFund(uniqueId, fund);
}


int Supplier::request(ItemType it, int qty) {
    // DONE
    supplier_mutex.lock();

    int bill = qty * getCostPerUnit(it);
    if (stocks[it] >= qty) {
        stocks[it] -= qty;
        money += bill;

        supplier_mutex.unlock();
        return bill;
    }

    supplier_mutex.unlock();
    return 0;
}

void Supplier::run() {
    interface->consoleAppendText(uniqueId, "[START] Supplier routine");
    while (!PcoThread::thisThread()->stopRequested()) {
        ItemType resourceSupplied = getRandomItemFromStock();
        int supplierCost = getEmployeeSalary(getEmployeeThatProduces(resourceSupplied));


        supplier_mutex.lock();

        // DONE
        if (money >= supplierCost) {

            /* Temps aléatoire borné qui simule l'attente du travail fini*/
            interface->simulateWork();
            //DONE

            money -= supplierCost;
            stocks[resourceSupplied] += 1;
            ++nbSupplied;
        }

        supplier_mutex.unlock();


        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Supplier routine");
}


std::map<ItemType, int> Supplier::getItemsForSale() {
    return stocks;
}

int Supplier::getMaterialCost() {
    int totalCost = 0;
    for (const auto &item : resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

int Supplier::getAmountPaidToWorkers() {
    return nbSupplied * getEmployeeSalary(EmployeeType::Supplier);
}

void Supplier::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

std::vector<ItemType> Supplier::getResourcesSupplied() const {
    return resourcesSupplied;
}

int Supplier::send(ItemType it, int qty, int bill) {
    return 0;
}
