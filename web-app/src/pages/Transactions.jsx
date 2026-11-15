// Transactions: add, search, sort, delete and undo. Searching and sorting are done by
// the C++ program (linear search / bubble sort); this page only shows the result.
import { useState } from 'react';
import { ArrowLeftRight, Plus, Search, Trash2, TriangleAlert, Undo2, X } from 'lucide-react';
import { api } from '../api.js';
import { useData } from '../useData.js';
import { money } from '../format.js';
import { CategoryIcon, EmptyState, PageHeader } from '../components/Common.jsx';
import { useToast } from '../components/Toast.jsx';
import Modal from '../components/Modal.jsx';
import { TransactionForm } from '../components/Forms.jsx';
import { showBudgetToast } from '../components/budgetToast.js';

export default function Transactions() {
  const [search, setSearch] = useState(null);          // null = show all, else { by, value }
  const [searchBy, setSearchBy] = useState('category');
  const [searchText, setSearchText] = useState('');
  const [adding, setAdding] = useState(false);
  const toast = useToast();

  const { data, error, reload } = useData(async () => {
    const all = await api('/api/transactions');
    if (!search) return { list: all.transactions, canUndo: all.canUndo };
    const found = await api(`/api/transactions/search?by=${encodeURIComponent(search.by)}&value=${encodeURIComponent(search.value)}`);
    return { list: found.transactions, canUndo: all.canUndo };
  }, [search]);   // load again whenever the search changes

  // Runs an action and turns any error from the server into a toast.
  const run = async action => {
    try { await action(); } catch (err) { toast(err.message, 'error'); }
  };

  const undo = () => run(async () => {
    await api('/api/transactions/undo', {});
    toast('Last action undone.');
    await reload();
  });

  const remove = t => run(async () => {
    await api('/api/transactions/delete', { id: t.id });
    toast(`Deleted transaction ${t.id} (${t.category}, ${money(t.amount)}).`, 'ok', { label: 'Undo', onClick: undo });
    await reload();
  });

  const sort = by => run(async () => {
    await api('/api/transactions/sort', { by });
    toast(`Sorted by ${by}.`);
    await reload();
  });

  const addTransaction = async values => {
    const result = await api('/api/transactions/add', values);
    setAdding(false);
    showBudgetToast(toast, result, `Transaction added (ID ${result.id}).`);
    await reload();
  };

  const startSearch = event => {
    event.preventDefault();
    if (!searchText.trim()) return;
    setSearch({ by: searchBy, value: searchText.trim() });
  };
  const clearSearch = () => { setSearch(null); setSearchText(''); };

  if (error) return <EmptyState icon={TriangleAlert} title="Could not load transactions" text={error} />;
  if (!data) return <p className="loading">Loading…</p>;

  const income = data.list.filter(t => t.type === 'Income').reduce((sum, t) => sum + t.amount, 0);
  const expense = data.list.filter(t => t.type === 'Expense').reduce((sum, t) => sum + t.amount, 0);

  return (
    <>
      <PageHeader title="Transactions" subtitle="Every income and expense, from Cash or Bank">
        <button className="button secondary" onClick={undo} disabled={!data.canUndo}><Undo2 size={18} /> Undo</button>
        <button className="button primary" onClick={() => setAdding(true)}><Plus size={18} /> Add transaction</button>
      </PageHeader>

      <section className="card">
        <div className="toolbar">
          <form className="search" onSubmit={startSearch} role="search">
            <select value={searchBy} onChange={event => setSearchBy(event.target.value)} aria-label="Search by">
              <option value="category">Category</option>
              <option value="type">Type</option>
              <option value="id">ID</option>
            </select>
            <div className="search-input">
              <Search size={16} />
              <input value={searchText} onChange={event => setSearchText(event.target.value)}
                     placeholder={searchBy === 'type' ? 'Income or Expense' : searchBy === 'id' ? 'e.g. 12' : 'e.g. Food'}
                     aria-label="Search for" />
            </div>
            <button className="button primary" type="submit">Search</button>
            {search && <button className="button secondary" type="button" onClick={clearSearch}><X size={16} /> Clear</button>}
          </form>
          <label className="sort">
            <span>Sort by</span>
            <select defaultValue="" onChange={event => { if (event.target.value) sort(event.target.value); event.target.value = ''; }} aria-label="Sort by">
              <option value="" disabled>Choose…</option>
              <option value="date">Date (oldest first)</option>
              <option value="amount">Amount (smallest first)</option>
              <option value="category">Category (A–Z)</option>
            </select>
          </label>
        </div>

        <div className="chips">
          <span className="chip">{search ? `Results for ${search.by} "${search.value}"` : 'All transactions'}: {data.list.length}</span>
          <span className="chip good">In {money(income)}</span>
          <span className="chip bad">Out {money(expense)}</span>
        </div>

        {data.list.length === 0 ? (
          <EmptyState icon={ArrowLeftRight} title={search ? 'No matching transactions' : 'No transactions yet'}
                      text={search ? 'Try another search.' : 'Add your first income or expense.'} />
        ) : (
          <div className="table-wrap">
            <table className="table">
              <thead>
                <tr><th>Category</th><th>Description</th><th>Date</th><th>Account</th><th className="num">Amount</th><th className="num">ID</th><th /></tr>
              </thead>
              <tbody>
                {data.list.map(t => (
                  <tr key={t.id}>
                    <td><div className="cell-category"><CategoryIcon category={t.category} size={34} /><span>{t.category}</span></div></td>
                    <td className="muted">{t.description || '—'}</td>
                    <td>{t.date}</td>
                    <td><span className={'badge ' + t.account.toLowerCase()}>{t.account}</span></td>
                    <td className={'num strong ' + (t.type === 'Income' ? 'good' : 'bad')}>{t.type === 'Income' ? '+' : '−'}{money(t.amount)}</td>
                    <td className="num muted">{t.id}</td>
                    <td className="num">
                      <button className="icon-button danger" aria-label={`Delete transaction ${t.id}`} onClick={() => remove(t)}><Trash2 size={17} /></button>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </section>

      {adding && (
        <Modal title="Add transaction" onClose={() => setAdding(false)}>
          <TransactionForm onSubmit={addTransaction} onCancel={() => setAdding(false)} />
        </Modal>
      )}
    </>
  );
}
