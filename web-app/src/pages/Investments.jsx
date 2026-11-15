// Investments: the C++ Investment classes (Stock, MutualFund, FixedDeposit, Gold, Other)
// calculate profit/loss; the type name comes from the virtual getType() (polymorphism).
import { useState } from 'react';
import { Cell, Pie, PieChart, ResponsiveContainer, Tooltip, Legend } from 'recharts';
import { Pencil, Plus, TrendingDown, TrendingUp, TriangleAlert } from 'lucide-react';
import { api } from '../api.js';
import { useData } from '../useData.js';
import { money } from '../format.js';
import { EmptyState, PageHeader, PALETTE } from '../components/Common.jsx';
import { useToast } from '../components/Toast.jsx';
import Modal from '../components/Modal.jsx';
import { INVESTMENT_TYPES, InvestmentForm, SingleValueForm } from '../components/Forms.jsx';

const tooltipStyle = { contentStyle: { background: 'var(--surface)', border: '1px solid var(--border)', borderRadius: 12, color: 'var(--text)' } };

export default function Investments() {
  const { data, error, reload } = useData(() => api('/api/investments'));
  const [adding, setAdding] = useState(false);
  const [editing, setEditing] = useState(null);
  const toast = useToast();

  if (error) return <EmptyState icon={TriangleAlert} title="Could not load investments" text={error} />;
  if (!data) return <p className="loading">Loading…</p>;
  const list = data.investments;

  const invested = list.reduce((sum, i) => sum + i.invested, 0);
  const current = list.reduce((sum, i) => sum + i.current, 0);
  const profit = current - invested;
  // Current value per type, for the allocation chart.
  const byType = INVESTMENT_TYPES
    .map(type => ({ type, value: list.filter(i => i.type === type).reduce((sum, i) => sum + i.current, 0) }))
    .filter(t => t.value > 0);

  const addInvestment = async values => {
    await api('/api/investments/add', values);
    setAdding(false);
    toast(`${values.type} "${values.name}" added.`);
    await reload();
  };

  const updateValue = async value => {
    await api('/api/investments/update', { number: editing.number, value });
    setEditing(null);
    toast('Current value updated.');
    await reload();
  };

  return (
    <>
      <PageHeader title="Investments" subtitle="Stocks, mutual funds, fixed deposits, gold and more">
        <button className="button primary" onClick={() => setAdding(true)}><Plus size={18} /> Add investment</button>
      </PageHeader>

      {list.length === 0 ? (
        <section className="card"><EmptyState icon={TrendingUp} title="No investments yet" text="Add a stock, FD, gold or mutual fund to track profit and loss." /></section>
      ) : (
        <>
          <div className="grid-2">
            <section className="card portfolio">
              <p className="muted">Portfolio value</p>
              <p className="balance-value">{money(current)}</p>
              <p className="muted">Invested {money(invested)}</p>
              <p className={'pl-pill ' + (profit >= 0 ? 'good' : 'bad')}>
                {profit >= 0 ? <TrendingUp size={16} /> : <TrendingDown size={16} />}
                {profit >= 0 ? '+' : '−'}{money(Math.abs(profit))} ({invested ? (profit / invested * 100).toFixed(2) : '0.00'}%)
              </p>
            </section>
            <section className="card">
              <h2>Allocation by type</h2>
              <ResponsiveContainer width="100%" height={200}>
                <PieChart>
                  <Pie data={byType} dataKey="value" nameKey="type" innerRadius={50} outerRadius={80} paddingAngle={2} stroke="none">
                    {byType.map((t, i) => <Cell key={t.type} fill={PALETTE[i % PALETTE.length]} />)}
                  </Pie>
                  <Tooltip formatter={value => money(value)} {...tooltipStyle} />
                  <Legend iconType="circle" layout="vertical" align="right" verticalAlign="middle" wrapperStyle={{ fontSize: 13 }} />
                </PieChart>
              </ResponsiveContainer>
            </section>
          </div>

          <section className="card">
            <div className="table-wrap">
              <table className="table">
                <thead>
                  <tr><th>Name</th><th>Type</th><th className="num">Quantity</th><th className="num">Invested</th><th className="num">Current value</th><th className="num">Profit / loss</th><th /></tr>
                </thead>
                <tbody>
                  {list.map(i => (
                    <tr key={i.number}>
                      <td className="strong">{i.name}</td>
                      <td><span className="badge">{i.type}</span></td>
                      <td className="num">{i.quantity}</td>
                      <td className="num">{money(i.invested)}</td>
                      <td className="num">{money(i.current)}</td>
                      <td className={'num strong ' + (i.profitLoss >= 0 ? 'good' : 'bad')}>
                        {i.profitLoss >= 0 ? '+' : '−'}{money(Math.abs(i.profitLoss))}
                        <span className="small"> ({i.percent.toFixed(2)}%)</span>
                      </td>
                      <td className="num">
                        <button className="icon-button" aria-label={`Update value of ${i.name}`} onClick={() => setEditing(i)}><Pencil size={17} /></button>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </section>
        </>
      )}

      {adding && (
        <Modal title="Add investment" onClose={() => setAdding(false)}>
          <InvestmentForm onSubmit={addInvestment} onCancel={() => setAdding(false)} />
        </Modal>
      )}
      {editing && (
        <Modal title={`Update value: ${editing.name}`} onClose={() => setEditing(null)}>
          <SingleValueForm label="Current value (₹)" initial={editing.current} submitLabel="Save value"
                           onSubmit={updateValue} onCancel={() => setEditing(null)} />
        </Modal>
      )}
    </>
  );
}
