// The forms shown inside pop-ups. Each one collects values and calls onSubmit(values).
// If the C++ server refuses (e.g. 31-02-2026, not enough money), its message is shown
// inside the form and the values stay, so the user can fix them.
import { useState } from 'react';
import { Field } from './Common.jsx';
import { MONTH_NAMES, toAppDate, todayIso } from '../format.js';

// Note on setV(prev => ...): every change is built on the LATEST values (prev), not on the
// values from the last time the form was drawn. Otherwise two quick changes (e.g. clicking
// "Income" and then autofill) would overwrite each other.

// Shared submit logic: prevents double clicks and shows the server's error message.
function useSubmit(onSubmit) {
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState('');
  const submit = async (event, values) => {
    event.preventDefault();
    setBusy(true);
    setError('');
    try {
      await onSubmit(values);
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  };
  return { busy, error, submit };
}

function FormFooter({ busy, error, label, onCancel }) {
  return (
    <>
      {error && <p className="form-error" role="alert">{error}</p>}
      <div className="form-footer">
        <button type="button" className="button secondary" onClick={onCancel}>Cancel</button>
        <button type="submit" className="button primary" disabled={busy}>{busy ? 'Saving…' : label}</button>
      </div>
    </>
  );
}

// Used for normal transactions AND pending (planned) transactions.
export function TransactionForm({ onSubmit, onCancel, submitLabel = 'Add transaction' }) {
  const [v, setV] = useState({ type: 'Expense', amount: '', category: '', date: todayIso(), account: 'Cash', description: '' });
  const set = name => event => { const value = event.target.value; setV(prev => ({ ...prev, [name]: value })); };
  const { busy, error, submit } = useSubmit(onSubmit);

  return (
    <form onSubmit={event => submit(event, { ...v, date: toAppDate(v.date) })}>
      <div className="segmented" role="radiogroup" aria-label="Type">
        {['Expense', 'Income'].map(type => (
          <button type="button" key={type} role="radio" aria-checked={v.type === type}
                  className={v.type === type ? `active ${type.toLowerCase()}` : ''}
                  onClick={() => setV(prev => ({ ...prev, type }))}>{type}</button>
        ))}
      </div>
      <div className="form-grid">
        <Field label="Amount (₹)" name="amount" type="number" step="0.01" min="0.01" required value={v.amount} onChange={set('amount')} />
        <Field label="Category" name="category" required placeholder="Food, Rent, Salary…" value={v.category} onChange={set('category')} />
        <Field label="Date" name="date" type="date" required value={v.date} onChange={set('date')} />
        <Field label="Account">
          <select name="account" value={v.account} onChange={set('account')}>
            <option>Cash</option><option>Bank</option>
          </select>
        </Field>
        <Field label="Description (optional)" name="description" value={v.description} onChange={set('description')} />
      </div>
      <FormFooter busy={busy} error={error} label={submitLabel} onCancel={onCancel} />
    </form>
  );
}

export function BudgetForm({ onSubmit, onCancel }) {
  const now = new Date();
  const [v, setV] = useState({ category: '', limit: '', month: String(now.getMonth() + 1), year: String(now.getFullYear()) });
  const set = name => event => { const value = event.target.value; setV(prev => ({ ...prev, [name]: value })); };
  const { busy, error, submit } = useSubmit(onSubmit);

  return (
    <form onSubmit={event => submit(event, v)}>
      <div className="form-grid">
        <Field label="Category" name="category" required value={v.category} onChange={set('category')} />
        <Field label="Monthly limit (₹)" name="limit" type="number" step="0.01" min="0.01" required value={v.limit} onChange={set('limit')} />
        <Field label="Month">
          <select name="month" value={v.month} onChange={set('month')}>
            {MONTH_NAMES.map((name, i) => <option key={name} value={i + 1}>{name}</option>)}
          </select>
        </Field>
        <Field label="Year" name="year" type="number" min="1" required value={v.year} onChange={set('year')} />
      </div>
      <p className="hint">A budget only counts expenses dated in its own month.</p>
      <FormFooter busy={busy} error={error} label="Add budget" onCancel={onCancel} />
    </form>
  );
}

export function GoalForm({ suggestedId, onSubmit, onCancel }) {
  const [v, setV] = useState({ id: String(suggestedId), name: '', target: '', savings: '0', monthly: '', date: '' });
  const set = name => event => { const value = event.target.value; setV(prev => ({ ...prev, [name]: value })); };
  const { busy, error, submit } = useSubmit(onSubmit);

  return (
    <form onSubmit={event => submit(event, { ...v, date: toAppDate(v.date) })}>
      <div className="form-grid">
        <Field label="Goal name" name="name" required placeholder="Laptop, Trip…" value={v.name} onChange={set('name')} />
        <Field label="Goal ID" name="id" type="number" min="1" step="1" required value={v.id} onChange={set('id')} />
        <Field label="Target amount (₹)" name="target" type="number" step="0.01" min="0.01" required value={v.target} onChange={set('target')} />
        <Field label="Saved so far (₹)" name="savings" type="number" step="0.01" min="0" required value={v.savings} onChange={set('savings')} />
        <Field label="Saving per month (₹)" name="monthly" type="number" step="0.01" min="0" required value={v.monthly} onChange={set('monthly')} />
        <Field label="Target date" name="date" type="date" required value={v.date} onChange={set('date')} />
      </div>
      <FormFooter busy={busy} error={error} label="Add goal" onCancel={onCancel} />
    </form>
  );
}

export function GoalUpdateForm({ goal, onSubmit, onCancel }) {
  const [v, setV] = useState({ savings: String(goal.savings), monthly: String(goal.monthly) });
  const set = name => event => { const value = event.target.value; setV(prev => ({ ...prev, [name]: value })); };
  const { busy, error, submit } = useSubmit(onSubmit);

  return (
    <form onSubmit={event => submit(event, v)}>
      <div className="form-grid">
        <Field label="Saved so far (₹)" name="savings" type="number" step="0.01" min="0" required value={v.savings} onChange={set('savings')} />
        <Field label="Saving per month (₹)" name="monthly" type="number" step="0.01" min="0" required value={v.monthly} onChange={set('monthly')} />
      </div>
      <FormFooter busy={busy} error={error} label="Update goal" onCancel={onCancel} />
    </form>
  );
}

export const INVESTMENT_TYPES = ['Stock', 'Mutual Fund', 'Fixed Deposit', 'Gold', 'Other'];

export function InvestmentForm({ onSubmit, onCancel }) {
  const [v, setV] = useState({ type: 'Stock', name: '', invested: '', current: '', quantity: '' });
  const set = name => event => { const value = event.target.value; setV(prev => ({ ...prev, [name]: value })); };
  const { busy, error, submit } = useSubmit(onSubmit);

  return (
    <form onSubmit={event => submit(event, v)}>
      <div className="form-grid">
        <Field label="Type">
          <select name="type" value={v.type} onChange={set('type')}>
            {INVESTMENT_TYPES.map(type => <option key={type}>{type}</option>)}
          </select>
        </Field>
        <Field label="Name" name="name" required placeholder="TCS, SBI FD…" value={v.name} onChange={set('name')} />
        <Field label="Amount invested (₹)" name="invested" type="number" step="0.01" min="0.01" required value={v.invested} onChange={set('invested')} />
        <Field label="Current value (₹)" name="current" type="number" step="0.01" min="0" required value={v.current} onChange={set('current')} />
        <Field label="Quantity (shares / units / grams)" name="quantity" type="number" step="any" min="0.0001" required value={v.quantity} onChange={set('quantity')} />
      </div>
      <FormFooter busy={busy} error={error} label="Add investment" onCancel={onCancel} />
    </form>
  );
}

export function BalanceForm({ cash, bank, onSubmit, onCancel }) {
  const [v, setV] = useState({ cash: String(cash), bank: String(bank) });
  const set = name => event => { const value = event.target.value; setV(prev => ({ ...prev, [name]: value })); };
  const { busy, error, submit } = useSubmit(onSubmit);

  return (
    <form onSubmit={event => submit(event, v)}>
      <p className="hint">Use this for your starting balances. It replaces the current Cash and Bank balances.</p>
      <div className="form-grid">
        <Field label="Cash (₹)" name="cash" type="number" step="0.01" min="0" required value={v.cash} onChange={set('cash')} />
        <Field label="Bank (₹)" name="bank" type="number" step="0.01" min="0" required value={v.bank} onChange={set('bank')} />
      </div>
      <FormFooter busy={busy} error={error} label="Set balances" onCancel={onCancel} />
    </form>
  );
}

// One number in a small pop-up: used for "change budget limit" and "update investment value".
export function SingleValueForm({ label, initial, submitLabel, onSubmit, onCancel }) {
  const [value, setValue] = useState(String(initial));
  const { busy, error, submit } = useSubmit(onSubmit);

  return (
    <form onSubmit={event => submit(event, value)}>
      <div className="form-grid">
        <Field label={label} name="value" type="number" step="0.01" min="0" required value={value} onChange={event => setValue(event.target.value)} />
      </div>
      <FormFooter busy={busy} error={error} label={submitLabel} onCancel={onCancel} />
    </form>
  );
}
