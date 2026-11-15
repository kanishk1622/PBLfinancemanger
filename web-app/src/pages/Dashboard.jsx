// Dashboard: balances, charts, budgets, goals and the latest transactions.
import { useState } from 'react';
import {
  Bar, BarChart, CartesianGrid, Cell, Legend, Pie, PieChart, ResponsiveContainer, Tooltip, XAxis, YAxis,
} from 'recharts';
import { ArrowDownRight, ArrowUpRight, Landmark, Plus, Target, TrendingUp, TriangleAlert, Wallet, Banknote, PiggyBank } from 'lucide-react';
import { api } from '../api.js';
import { useData } from '../useData.js';
import { dateKey, money, monthLabel, shortMoney } from '../format.js';
import { CategoryIcon, EmptyState, PageHeader, PALETTE, ProgressBar, categoryColor } from '../components/Common.jsx';
import { useToast } from '../components/Toast.jsx';
import Modal from '../components/Modal.jsx';
import { BalanceForm, TransactionForm } from '../components/Forms.jsx';
import { showBudgetToast } from '../components/budgetToast.js';

async function loadAll() {
  const [dashboard, transactions, budgets, goals] = await Promise.all([
    api('/api/dashboard'), api('/api/transactions'), api('/api/budgets'), api('/api/goals'),
  ]);
  return { d: dashboard, transactions: transactions.transactions, budgets: budgets.budgets, goals: goals.goals };
}

// Groups transactions by month for the bar chart (display only; the totals come from
// the transactions the C++ program sent). Shows the last 6 months that have data.
function monthlyTotals(transactions) {
  const months = new Map();
  for (const t of transactions) {
    const [, month, year] = t.date.split('-');
    const key = year + month;
    if (!months.has(key)) months.set(key, { key, label: monthLabel(Number(month), year), Income: 0, Expense: 0 });
    months.get(key)[t.type] += t.amount;
  }
  return [...months.values()].sort((a, b) => a.key.localeCompare(b.key)).slice(-6);
}

function StatCard({ label, value, icon: Icon, tone }) {
  return (
    <div className="card stat">
      <span className={`stat-icon ${tone}`}><Icon size={20} /></span>
      <div>
        <p className="stat-label">{label}</p>
        <p className={`stat-value ${tone}`}>{value}</p>
      </div>
    </div>
  );
}

const tooltipStyle = { contentStyle: { background: 'var(--surface)', border: '1px solid var(--border)', borderRadius: 12, color: 'var(--text)' } };

export default function Dashboard({ onNavigate }) {
  const { data, error, reload } = useData(loadAll);
  const [adding, setAdding] = useState(false);
  const [settingBalances, setSettingBalances] = useState(false);
  const toast = useToast();

  if (error) return <EmptyState icon={TriangleAlert} title="Could not load the dashboard" text={error} />;
  if (!data) return <p className="loading">Loading…</p>;
  const { d, transactions, budgets, goals } = data;

  const addTransaction = async values => {
    const result = await api('/api/transactions/add', values);
    setAdding(false);
    showBudgetToast(toast, result, `Transaction added (ID ${result.id}).`);
    reload();
  };

  const setBalances = async values => {
    await api('/api/balances', values);
    setSettingBalances(false);
    toast('Balances set.');
    reload();
  };

  const recent = [...transactions].sort((a, b) => dateKey(b.date) - dateKey(a.date) || b.id - a.id).slice(0, 5);
  const monthly = monthlyTotals(transactions);
  const spending = [...d.spendingByCategory].sort((a, b) => b.amount - a.amount);

  return (
    <>
      <PageHeader title="Dashboard" subtitle="Your money at a glance">
        <button className="button secondary" onClick={() => setSettingBalances(true)}><Landmark size={18} /> Set balances</button>
        <button className="button primary" onClick={() => setAdding(true)}><Plus size={18} /> Add transaction</button>
      </PageHeader>

      <div className="hero-grid">
        <div className="card balance-card">
          <p className="balance-label">Total balance</p>
          <p className="balance-value">{money(d.total)}</p>
          <div className="balance-split">
            <span><Banknote size={16} /> Cash <strong>{money(d.cash)}</strong></span>
            <span><Landmark size={16} /> Bank <strong>{money(d.bank)}</strong></span>
          </div>
        </div>
        <div className="stat-grid">
          <StatCard label="Total income" value={money(d.income)} icon={ArrowDownRight} tone="good" />
          <StatCard label="Total expense" value={money(d.expense)} icon={ArrowUpRight} tone="bad" />
          <StatCard label="Net savings" value={money(d.net)} icon={Wallet} tone={d.net >= 0 ? 'good' : 'bad'} />
          <StatCard label="Investment profit/loss" value={money(d.profitLoss)} icon={TrendingUp} tone={d.profitLoss >= 0 ? 'good' : 'bad'} />
        </div>
      </div>

      {d.exceededBudgets.map(name => (
        <div key={name} className="alert"><TriangleAlert size={18} /> Budget exceeded: {name}</div>
      ))}

      <div className="grid-2">
        <section className="card">
          <h2>Spending by category</h2>
          {spending.length === 0 ? <EmptyState title="No expenses yet" /> : (
            <div className="donut-wrap">
              <ResponsiveContainer width="100%" height={260}>
                <PieChart>
                  <Pie data={spending} dataKey="amount" nameKey="category" innerRadius={70} outerRadius={105} paddingAngle={2} stroke="none">
                    {/* each slice gets the next palette colour, so no two slices look the same */}
                    {spending.map((s, i) => <Cell key={s.category} fill={PALETTE[i % PALETTE.length]} />)}
                  </Pie>
                  <Tooltip formatter={value => money(value)} {...tooltipStyle} />
                  <Legend iconType="circle" wrapperStyle={{ fontSize: 13 }} />
                </PieChart>
              </ResponsiveContainer>
              <div className="donut-center"><span>Spent</span><strong>{shortMoney(d.expense)}</strong></div>
            </div>
          )}
        </section>

        <section className="card">
          <h2>Income vs expense by month</h2>
          {monthly.length === 0 ? <EmptyState title="No transactions yet" /> : (
            <div className="chart-text">
              <ResponsiveContainer width="100%" height={260}>
                <BarChart data={monthly} barGap={4}>
                  <CartesianGrid vertical={false} stroke="var(--border)" />
                  <XAxis dataKey="label" tick={{ fill: 'currentColor', fontSize: 12 }} axisLine={false} tickLine={false} />
                  <YAxis tickFormatter={shortMoney} tick={{ fill: 'currentColor', fontSize: 12 }} axisLine={false} tickLine={false} width={56} />
                  <Tooltip formatter={value => money(value)} cursor={{ fill: 'var(--hover)' }} {...tooltipStyle} />
                  <Legend iconType="circle" wrapperStyle={{ fontSize: 13 }} />
                  <Bar dataKey="Income" fill="#22c55e" radius={[6, 6, 0, 0]} />
                  <Bar dataKey="Expense" fill="#ef4444" radius={[6, 6, 0, 0]} />
                </BarChart>
              </ResponsiveContainer>
            </div>
          )}
        </section>
      </div>

      <div className="grid-2">
        <section className="card">
          <div className="card-head"><h2>Budgets</h2><button className="link" onClick={() => onNavigate('budgets')}>View all</button></div>
          {budgets.length === 0 ? <EmptyState icon={PiggyBank} title="No budgets yet" /> : budgets.slice(0, 4).map(b => (
            <div key={b.category + b.month + b.year} className="mini-row">
              <div className="mini-head">
                <span>{b.category} <span className="muted">· {monthLabel(b.month, b.year)}</span></span>
                <span className={b.remaining < 0 ? 'bad' : 'muted'}>{money(b.spent)} / {money(b.limit)}</span>
              </div>
              <ProgressBar percent={b.percent} over={b.remaining < 0} color={categoryColor(b.category)} />
            </div>
          ))}
        </section>

        <section className="card">
          <div className="card-head"><h2>Goals</h2><button className="link" onClick={() => onNavigate('goals')}>View all</button></div>
          {goals.length === 0 ? <EmptyState icon={Target} title="No goals yet" /> : goals.slice(0, 4).map(g => (
            <div key={g.number} className="mini-row">
              <div className="mini-head">
                <span>{g.name}</span>
                <span className="muted">{g.progress.toFixed(0)}% of {money(g.target)}</span>
              </div>
              <ProgressBar percent={g.progress} color="var(--accent)" />
            </div>
          ))}
        </section>
      </div>

      <section className="card">
        <div className="card-head"><h2>Recent transactions</h2><button className="link" onClick={() => onNavigate('transactions')}>View all</button></div>
        {recent.length === 0 ? <EmptyState title="No transactions yet" text="Add your first income or expense." /> : (
          <ul className="tx-list">
            {recent.map(t => (
              <li key={t.id} className="tx-row">
                <CategoryIcon category={t.category} />
                <div className="tx-main">
                  <span className="tx-title">{t.category}</span>
                  <span className="muted">{t.description || t.account} · {t.date}</span>
                </div>
                <span className={'tx-amount ' + (t.type === 'Income' ? 'good' : 'bad')}>
                  {t.type === 'Income' ? '+' : '−'}{money(t.amount)}
                </span>
              </li>
            ))}
          </ul>
        )}
      </section>

      {adding && (
        <Modal title="Add transaction" onClose={() => setAdding(false)}>
          <TransactionForm onSubmit={addTransaction} onCancel={() => setAdding(false)} />
        </Modal>
      )}
      {settingBalances && (
        <Modal title="Set account balances" onClose={() => setSettingBalances(false)}>
          <BalanceForm cash={d.cash} bank={d.bank} onSubmit={setBalances} onCancel={() => setSettingBalances(false)} />
        </Modal>
      )}
    </>
  );
}
