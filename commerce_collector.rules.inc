<?php

/**
 * @file
 * Rules integration for Commerce Collector module.
 */


/**
 * Implements hook_rules_action_info().
 */
function commerce_collector_rules_action_info() {
  $actions = array();

  $actions['commerce_collector_activate'] = array(
    'label' => t('Activate collector payments on commerce order'),
    'parameter' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Order to activate'),
      ),
    ),
    'group' => t('Commerce Payment'),
    'callbacks' => array(
      'execute' => 'commerce_collector_rules_activate',
    ),
  );

  $actions['commerce_collector_deactivate'] = array(
    'label' => t('Deactivate collector payments on commerce order'),
    'parameter' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Order to deactivate'),
      ),
    ),
    'group' => t('Commerce Payment'),
    'callbacks' => array(
      'execute' => 'commerce_collector_rules_deactivate',
    ),
  );

  return $actions;
}

/**
 * Activate all pending request.
 *
 * @param $order
 *   Order object.
 */
function commerce_collector_rules_activate($order) {

  // Include commerce_collector.inc file.
  module_load_include('inc', 'commerce_collector', 'commerce_collector');

  // Find all invoices which can be activated.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'commerce_payment_transaction')
    ->propertyCondition('order_id', $order->order_id)
    ->propertyCondition('payment_method', 'commerce_collector')
    ->propertyCondition('remote_status', COMMERCE_COLLECTOR_PAYMENT_STATUS_PENDING);
  $result = $query->execute();

  // Inform only users with update perm to payment transaction.
  $inform = commerce_payment_transaction_order_access('update', $order);

  if (isset($result['commerce_payment_transaction']) && count($result['commerce_payment_transaction']) == 1) {

    // Get transaction id.
    $transaction_ids = array_keys($result['commerce_payment_transaction']);

    // Load transaction entity.
    $transaction = commerce_payment_transaction_load(reset($transaction_ids));

    // Process, activate transaction.
    commerce_collector_activate_invoice($transaction);
  }
  elseif (isset($result['commerce_payment_transaction']) && count($result['commerce_payment_transaction']) > 1) {
    if ($inform) {
      drupal_set_message(t('Unable to auto-activate more than one invoice at the same time. <a href="@href">Manage it manually.</a>', array('@href' => url('admin/commerce/orders/' . $order->order_id . '/payment'))), 'warning');
    }
  }
}

/**
 * Deactivate all active and pending requests.
 *
 * If invoice could not be canceled it will be credited instead.
 *
 * @param $order
 *   Order object.
 */
function commerce_collector_rules_deactivate($order) {

  // Include commerce_collector.inc file.
  module_load_include('inc', 'commerce_collector', 'commerce_collector');

  // First of all - cancel pending transactions not registered by Collector.
  _commerce_collector_cancel_deprecated_transactions($order);

  // Find all invoices which can be activated.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'commerce_payment_transaction')
    ->propertyCondition('order_id', $order->order_id)
    ->propertyCondition('payment_method', 'commerce_collector')
    ->propertyCondition('status', COMMERCE_PAYMENT_STATUS_FAILURE, '!=')
    ->propertyCondition('remote_status', COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED, '!=');
  $result = $query->execute();

  // Inform only users with update perm to payment transaction.
  $inform = commerce_payment_transaction_order_access('update', $order);

  if (isset($result['commerce_payment_transaction']) && count($result['commerce_payment_transaction']) == 1) {

    // Get transaction id.
    $transaction_ids = array_keys($result['commerce_payment_transaction']);

    // Load transaction entity.
    $transaction = commerce_payment_transaction_load(reset($transaction_ids));

    // Process, activate transaction.
    commerce_collector_deactivate_invoice($transaction);
  }
  elseif (isset($result['commerce_payment_transaction']) && count($result['commerce_payment_transaction']) > 1) {
    if ($inform) {
      drupal_set_message(t('Unable to auto-deactivate more than one invoice at the same time. <a href="@href">Manage it manually.</a>', array('@href' => url('admin/commerce/orders/' . $order->order_id . '/payment'))), 'warning');
    }
  }
}
