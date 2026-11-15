// Budgets: a spending limit per category per month. "Spent" is calculated by the C++
// Budget class from the transactions (only expenses dated in that month count).
import { useState } from 'react';
import { Pencil, PiggyBank, Plus, TriangleAlert } from 'lucide-react';
import { api } from '../api.js';
import { useData } from '../useData.js';
import { money, monthLabel } from '../format.js';
import { CategoryIcon, EmptyState, PageHeader, ProgressBar, categoryColor } from '../components/Common.jsx';
import { useToast } from '../components/Toast.jsx';
import Modal from '../components/Modal.jsx';
import { BudgetForm, SingleValueForm } from '../components/Forms.jsx';

export default function Budgets() {
  const { data, error, reload } = useData(() => api('/api/budgets'));
  const [adding, setAdding] = useState(false);
  const [editing, setEditing] = useState(null);   // the budget whose limit is being changed
  const toast = useToast();

  if (error) return <EmptyState icon={TriangleAlert} title="Could not load budgets" text={error} />;
  if (!data) return <p className="loading">Loading…</p>;

  // Newest month first, then by category.
  const budgets = [...data.budgets].sort((a, b) => (b.year - a.year) || (b.month - a.month) || a.category.localeCompare(b.category));

  const addBudget = async values => {
    await api('/api/budgets/add', values);
    setAdding(false);
    toast('Budget added.');
    await reload();
  };

  const changeLimit = async limit => {
    await api('/api/budgets/update', { category: editing.category, month: editing.month, year: editing.year, limit });
    setEditing(null);
    toast('Budget limit changed.');
    await reload();
  };

  return (
    <>
      <PageHeader title="Budgets" subtitle="Monthly spending limits per category">
        <button className="button primary" onClick={() => setAdding(true)}><Plus size={18} /> Add budget</button>
      </PageHeader>

      {budgets.length === 0 ? (
        <section className="card">
          <EmptyState icon={PiggyBank} title="No budgets yet" text="Set a monthly limit, e.g. Food ₹5,000 for this month." />
        </section>
      ) : (
        <div className="card-grid">
          {budgets.map(b => {
            const over = b.remaining < 0;
            return (
              <section key={b.category + b.month + b.year} className={'card budget-card' + (over ? ' over' : '')}>
                <div className="card-head">
                  <div className="cell-category">
                    <CategoryIcon category={b.category} />
                    <div><p className="strong">{b.category}</p><p className="muted small">{monthLabel(b.month, b.year)}</p></div>
                  </div>
                  <button className="icon-button" aria-label={`Change limit of ${b.category}`} onClick={() => setEditing(b)}><Pencil size={17} /></button>
                </div>
                <p className="big-number">{money(b.spent)} <span className="muted small">of {money(b.limit)}</span></p>
                <ProgressBar percent={b.percent} over={over} color={categoryColor(b.category)} />
                <p className={'small ' + (over ? 'bad strong' : 'muted')}>
                  {over ? `Over budget by ${money(-b.remaining)}` : `${money(b.remaining)} left · ${b.percent.toFixed(0)}% used`}
                </p>
              </section>
            );
          })}
        </div>
      )}

      {adding && (
        <Modal title="Add monthly budget" onClose={() => setAdding(false)}>
          <BudgetForm onSubmit={addBudget} onCancel={() => setAdding(false)} />
        </Modal>
      )}
      {editing && (
        <Modal title={`Change limit: ${editing.category}, ${monthLabel(editing.month, editing.year)}`} onClose={() => setEditing(null)}>
          <SingleValueForm label="New monthly limit (₹)" initial={editing.limit} submitLabel="Save limit"
                           onSubmit={changeLimit} onCancel={() => setEditing(null)} />
        </Modal>
      )}
    </>
  );
}
