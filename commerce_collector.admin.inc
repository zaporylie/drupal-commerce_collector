<?php
/**
 * @file
 * Page callbacks accessible for users with administrative role only.
 */

/**
 * Page callback to retrieve invoice via Collector API.
 */
function commerce_collector_admin_get_invoice($order, $transaction) {

  module_load_include('inc', 'commerce_collector', 'commerce_collector');

  $data = array(
    'CountryCode' => $transaction->data['country_code'],
    'InvoiceNo' => $transaction->remote_id,
  );

  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);

  try {
    $result = _commerce_collector_request($data, $payment_method, 'GetCurrentInvoice', 'InformationService.svc', 'InformationService');
    $transaction->data['response'] = (array) $result;
    commerce_payment_transaction_save($transaction);
    drupal_set_message(t('Information about corresponding invoice has been updated.'));
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    watchdog('commerce_collector', print_r($e, TRUE), array(), WATCHDOG_ERROR);
  }

  drupal_goto('admin/commerce/orders/' . $order->order_id . '/payment');
}

/**
 * Wrapper over commerce_collector_activate_invoice() function.
 */
function commerce_collector_admin_activate_invoice($order, $transaction) {

  module_load_include('inc', 'commerce_collector', 'commerce_collector');

  commerce_collector_activate_invoice($transaction);

  drupal_goto('admin/commerce/orders/' . $order->order_id . '/payment');
}

/**
 * Wrapper over commerce_collector_deactivate_invoice() function.
 */
function commerce_collector_admin_deactivate_invoice($order, $transaction) {

  module_load_include('inc', 'commerce_collector', 'commerce_collector');

  commerce_collector_deactivate_invoice($transaction);

  drupal_goto('admin/commerce/orders/' . $order->order_id . '/payment');
}
