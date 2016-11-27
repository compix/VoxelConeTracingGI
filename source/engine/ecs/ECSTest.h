#pragma once

class ECSTest
{
public:
    static void runTests();

private:
    static void testComponentTypeID();
    static void testGetEntitiesWithComponents();
    static void testEntityCreateDestroyValid();
    static void testHasGetAddRemoveComponent();
    static void testSystem();
};
