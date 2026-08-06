DualHeap-Database

Develop a class that acts as a database of companies that need to pay the VAT.

A company is identified by its tax_ID (unique string, case sensitive) or name and address (unique pair of strings, case insensitive).

The class has multiple methods:
- newCompany (name, addr, id ) adds a new record.
- cancelCompany (name, addr) / cancelCompany (id) removes the corresponding record,  identified by name and address, or by tax_id.
- invoice (name, addr, amount) / invoice ( id, amount ) add an amount to the income of the target company.
- auditCompany ( name, addr, sum ) / auditCompany ( id, sum ) searches the database for target company and fills the output parameter (sum) with the total revenue of that company.
- medianInvoice () searches for the median amount in the registered invoices. Only positively registered invoices are included in the computation.
- firstCompany ( name, addr ) iterates over the list of companies and list them in an alphabetical order, sorted by company name
- nextCompany (name, addr) is similar; it finds the company that follows the given company

My implementation uses a Dual-Vector and Binary Search to achieve O(logn) lookup for both lookup criteria and Dual-Heaping (one max-heap, one min-heap) for O(1) lookup of median value. 